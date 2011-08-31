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
#include "FrameworkConfigurationLocation.h"
#include <assert.h>

#define base CKindElement

CFrameworkConfigurationLocation::CFrameworkConfigurationLocation(const string& strName, const string& strKind) : base(strName, strKind)
{
}

// From IXmlSink
bool CFrameworkConfigurationLocation::fromXml(const CXmlElement& xmlElement, CXmlSerializingContext& serializingContext)
{
    _strPath = xmlElement.getAttributeString("Path");

    if (_strPath.empty()) {

        serializingContext.setError("Empty Path attribute in elememnt " + xmlElement.getPath());

        return false;
    }
    return true;
}

string CFrameworkConfigurationLocation::getFilePath(const string& strBaseFolder) const
{
    if (isPathRelative()) {

        return strBaseFolder + "/" + _strPath;
    }
    return _strPath;
}

bool CFrameworkConfigurationLocation::isPathRelative() const
{
    return _strPath[0] != '/';
}

string CFrameworkConfigurationLocation::getFolderPath(const string& strBaseFolder) const
{
    uint32_t uiSlashPos = _strPath.rfind('/', -1);

    string strFolderPath = ".";

    if (isPathRelative()) {

        if (uiSlashPos != (uint32_t)-1) {

            strFolderPath = _strPath.substr(0, uiSlashPos);

            return strBaseFolder + "/" + strFolderPath;

        } else {

            return strBaseFolder;
        }
    } else {

        assert(uiSlashPos != (uint32_t)-1);

        strFolderPath = _strPath.substr(0, uiSlashPos);

        return strFolderPath;
    }
}

#if 0
string CFrameworkConfigurationLocation::getFileName() const
{
    uint32_t uiSlashPos = _strPath.rfind('/', -1);

    if (uiSlashPos != (uint32_t)-1) {

        return _strPath.substr(uiSlashPos + 1, _strPath.size() - uiSlashPos - 1);
    } else {

        return _strPath;
    }
}
#endif
