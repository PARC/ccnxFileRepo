/*
 * Copyright (c) 2016, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Patent rights are not granted under this agreement. Patent rights are
 *       available under FRAND terms.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX or PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * @author Christopher A. Wood, Palo Alto Research Center (Xerox PARC)
 * @copyright 2016, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC).  All rights reserved.
 */
#include <stdio.h>

#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_NameSegmentNumber.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_IdentityFile.h>

#include "ccnxFileRepo_Common.h"

/**
 * The name of this application. This is what shows when you run the client or server with '-h'
 */
const char *ccnxFileRepoCommon_ProgramName = "CCNx 1.0 Single File Manifest Transfer";

/**
 * The server chunk size.
 */
const size_t ccnxFileRepoCommon_ServerChunkSize = 4096;

/**
 * The client streaming I/O buffer size.
 */
const size_t ccnxFileRepoCommon_ClientBufferSize = 16384; // 4*4K


PARCIdentity *
ccnxFileRepoCommon_CreateAndGetIdentity(const char *keystoreName,
                                        const char *keystorePassword,
                                        const char *subjectName)
{
    parcSecurity_Init();

    unsigned int keyLength = 1024;
    unsigned int validityDays = 30;

    bool success = parcPkcs12KeyStore_CreateFile(keystoreName, keystorePassword, subjectName, keyLength, validityDays);
    assertTrue(success,
               "parcPkcs12KeyStore_CreateFile('%s', '%s', '%s', %d, %d) failed.",
               keystoreName, keystorePassword, subjectName, keyLength, validityDays);

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    PARCIdentity *result = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    parcIdentityFile_Release(&identityFile);

    parcSecurity_Fini();

    return result;
}

CCNxPortalFactory *
ccnxFileRepoCommon_SetupPortalFactory(const char *keystoreName,
                                      const char *keystorePassword,
                                      const char *subjectName)
{
    PARCIdentity *identity = ccnxFileRepoCommon_CreateAndGetIdentity(keystoreName,
                                                                     keystorePassword,
                                                                     subjectName);
    CCNxPortalFactory *result = ccnxPortalFactory_Create(identity);
    parcIdentity_Release(&identity);

    return result;
}

int
ccnxFileRepoCommon_ProcessCommandLineArguments(int argc, char **argv,
                                               int *commandArgCount, char **commandArgs,
                                               bool *needToShowUsage, bool *shouldExit)
{
    int status = EXIT_SUCCESS;
    *commandArgCount = 0;
    *needToShowUsage = false;

    for (size_t i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
                case 'h': {
                    *needToShowUsage = true;
                    *shouldExit = true;
                    break;
                }
                default: { // Unexpected '-' option.
                    *needToShowUsage = true;
                    *shouldExit = true;
                    status = EXIT_FAILURE;
                    break;
                }
            }
        } else {
            // Not a '-' option, so save it as a command argument.
            commandArgs[(*commandArgCount)++] = arg;
        }
    }
    return status;
}
