/*
 * Copyright (c) 2011-2014, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "RemoteProcessorServer.h"
#include "ListeningSocket.h"
#include <iostream>
#include <memory>
#include <assert.h>
#include <poll.h>
#include <unistd.h>
#include <strings.h>
#include "RequestMessage.h"
#include "AnswerMessage.h"
#include "RemoteCommandHandler.h"

using std::string;

CRemoteProcessorServer::CRemoteProcessorServer(uint16_t uiPort, IRemoteCommandHandler* pCommandHandler) :
    _uiPort(uiPort), _pCommandHandler(pCommandHandler), _bIsStarted(false), _pListeningSocket(NULL), _ulThreadId(0)
{
    // Create inband pipe
    pipe(_aiInbandPipe);
}

CRemoteProcessorServer::~CRemoteProcessorServer()
{
    stop();
}

// State
bool CRemoteProcessorServer::start()
{
    assert(!_bIsStarted);

    // Create server socket
    _pListeningSocket = new CListeningSocket;

    if (!_pListeningSocket->listen(_uiPort)) {

        // Remove listening socket
        delete _pListeningSocket;
        _pListeningSocket = NULL;

        return false;
    }

    // Create thread
    pthread_create(&_ulThreadId, NULL, thread_func, this);

    // State
    _bIsStarted = true;

    return true;
}

void CRemoteProcessorServer::stop()
{
    // Check state
    if (!_bIsStarted) {

        return;
    }

    // Cause exiting of the thread
    uint8_t ucData = 0;
    write(_aiInbandPipe[1], &ucData, sizeof(ucData));

    // Join thread
    pthread_join(_ulThreadId, NULL);

    _bIsStarted = false;

    // Remove listening socket
    delete _pListeningSocket;
    _pListeningSocket = NULL;
}

bool CRemoteProcessorServer::isStarted() const
{
    return _bIsStarted;
}

// Thread
void* CRemoteProcessorServer::thread_func(void* pData)
{
    reinterpret_cast<CRemoteProcessorServer*>(pData)->run();

    return NULL;
}

void CRemoteProcessorServer::run()
{
    struct pollfd _aPollFds[2];

    bzero(_aPollFds, sizeof(_aPollFds));

    // Build poll elements
    _aPollFds[0].fd = _pListeningSocket->getFd();
    _aPollFds[1].fd = _aiInbandPipe[0];
    _aPollFds[0].events = POLLIN;
    _aPollFds[1].events = POLLIN;

    while (true) {

        poll(_aPollFds, 2, -1);

        if (_aPollFds[0].revents & POLLIN) {

            // New incoming connection
            handleNewConnection();
        }
        if (_aPollFds[1].revents & POLLIN) {

            // Consume exit request
            uint8_t ucData;
            read(_aiInbandPipe[0], &ucData, sizeof(ucData));

            // Exit
            return;
        }
    }
}

// New connection
void CRemoteProcessorServer::handleNewConnection()
{
    const std::auto_ptr<CSocket> clientSocket(_pListeningSocket->accept());

    if (clientSocket.get() == NULL) {

        return;
    }

    // Set timeout
    clientSocket->setTimeout(5000);

    // Process all incoming requests from the client
    while (true) {

        // Process requests
        // Create command message
        CRequestMessage requestMessage;

        string strError;
        ///// Receive command
        CRequestMessage::Result res;
        res = requestMessage.serialize(clientSocket.get(), false, strError);

        switch (res) {
        case CRequestMessage::error:
            std::cout << "Error while receiving message: " << strError << std::endl;
            // fall through
        case CRequestMessage::peerDisconnected:
            // Consider peer disconnection as normal, no log
            return; // Bail out
        case CRequestMessage::success:
            break; // No error, continue
        }

        // Actually process the request
        bool bSuccess;

        string strResult;

        if (_pCommandHandler) {

            bSuccess = _pCommandHandler->remoteCommandProcess(requestMessage, strResult);

        } else {

            strResult = "No handler!";

            bSuccess = false;
        }

        // Send back answer
        // Create answer message
        CAnswerMessage answerMessage(strResult, bSuccess);

        ///// Send answer
        res = answerMessage.serialize(clientSocket.get(), true, strError);

        switch (res) {
        case CRequestMessage::peerDisconnected:
            // Peer should not disconnect while waiting for an answer
            // Fall through to log the error and bail out
        case CRequestMessage::error:
            std::cout << "Error while receiving message: " << strError << std::endl;
            return; // Bail out
        case CRequestMessage::success:
            break; // No error, continue
        }
    }
}
