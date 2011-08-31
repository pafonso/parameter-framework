/* <auto_header>
 * <FILENAME>
 * 
 * INTEL CONFIDENTIAL
 * Copyright © 2011 Intel 
 * Corporation All Rights Reserved.
 * 
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its suppliers
 * or licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and licensors. The
 * Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed, or
 * disclosed in any way without Intel’s prior express written permission.
 * 
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by Intel in writing.
 * 
 *  AUTHOR: Patrick Benavoli (patrickx.benavoli@intel.com)
 * CREATED: 2011-06-01
 * UPDATED: 2011-07-27
 * 
 * 
 * </auto_header>
 */
#pragma once

#include <stdint.h>
#include "BinaryStream.h"

using namespace std;

class CParameterBlackboard
{
public:
    CParameterBlackboard();
    ~CParameterBlackboard();

    // Size
    void setSize(uint32_t uiSize);
    uint32_t getSize() const;

    // Single parameter access
    void write(const void* pvSrcData, uint32_t uiSize, uint32_t uiOffset, bool bBigEndian);
    void read(void* pvDstData, uint32_t uiSize, uint32_t uiOffset, bool bBigEndian) const;

    // Access from/to subsystems
    void rawRead(void* pvDstData, uint32_t uiSize, uint32_t uiOffset) const;
    void rawWrite(const void* pvDstData, uint32_t uiSize, uint32_t uiOffset);

    // Configuration handling
    void restoreFrom(const CParameterBlackboard* pFromBlackboard, uint32_t uiOffset);
    void saveTo(CParameterBlackboard* pToBlackboard, uint32_t uiOffset) const;

    // Serialization
    void serialize(CBinaryStream& binaryStream);
private:
    uint8_t* _puiData;
    uint32_t _uiSize;
};

