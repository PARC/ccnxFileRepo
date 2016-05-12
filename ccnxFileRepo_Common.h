/*
 * Copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX OR PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ################################################################################
 * #
 * # PATENT NOTICE
 * #
 * # This software is distributed under the BSD 2-clause License (see LICENSE
 * # file).  This BSD License does not make any patent claims and as such, does
 * # not act as a patent grant.  The purpose of this section is for each contributor
 * # to define their intentions with respect to intellectual property.
 * #
 * # Each contributor to this source code is encouraged to state their patent
 * # claims and licensing mechanisms for any contributions made. At the end of
 * # this section contributors may each make their own statements.  Contributor's
 * # claims and grants only apply to the pieces (source code, programs, text,
 * # media, etc) that they have contributed directly to this software.
 * #
 * # There is no guarantee that this section is complete, up to date or accurate. It
 * # is up to the contributors to maintain their portion of this section and up to
 * # the user of the software to verify any claims herein.
 * #
 * # Do not remove this header notification.  The contents of this section must be
 * # present in all distributions of the software.  You may only modify your own
 * # intellectual property statements.  Please provide contact information.
 * 
 * - Palo Alto Research Center, Inc
 * This software distribution does not grant any rights to patents owned by Palo
 * Alto Research Center, Inc (PARC). Rights to these patents are available via
 * various mechanisms. As of January 2016 PARC has committed to FRAND licensing any
 * intellectual property used by its contributions to this software. You may
 * contact PARC at cipo@parc.com for more information or visit http://www.ccnx.org
 */
/**
 * @author Christopher A. Wood, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */

#ifndef ccnxFileRepoCommon_h
#define ccnxFileRepoCommon_h

#include <stdint.h>

#include <parc/security/parc_Identity.h>

#include <ccnx/common/ccnx_Name.h>

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>

/**
 * See ccnxFileRepoCommon.h for the initilization of these constants.
 */
extern const char *ccnxFileRepoCommon_ProgramName;

/**
 * The server chunk size.
 */
extern const size_t ccnxFileRepoCommon_ServerChunkSize;

/**
 * The client streaming I/O buffer size.
 */
extern const size_t ccnxFileRepoCommon_ClientBufferSize;

/**
 * Creates and returns a new randomly generated Identity, which is required for signing.
 * In a real application, you would actually use a real Identity. The returned instance
 * must eventually be released by calling parcIdentity_Release().
 *
 * @param [in] keystoreName The name of the file to save the new identity.
 * @param [in] keystorePassword The password of the file holding the identity.
 * @param [in] subjectName The name of the owner of the identity.
 *
 *
 * @return A new, randomly generated, PARCIdentity instance.
 */
PARCIdentity *ccnxFileRepoCommon_CreateAndGetIdentity(const char *keystoreName,
                                                                const char *keystorePassword,
                                                                const char *subjectName);

/**
 * Initialize and return a new instance of CCNxPortalFactory. A randomly generated identity is
 * used to initialize the factory. The returned instance must eventually be released by calling
 * ccnxPortalFactory_Release().
 *
 * @param [in] keystoreName The name of the file to save the new identity.
 * @param [in] keystorePassword The password of the file holding the identity.
 * @param [in] subjectName The name of the owner of the identity.
 *
 * @return A new instance of a CCNxPortalFactory initialized with a randomly created identity.
 */
CCNxPortalFactory *ccnxFileRepoCommon_SetupPortalFactory(const char *keystoreName,
                                                                   const char *keystorePassword,
                                                                   const char *subjectName);

/**
 * Process our command line arguments. If we're given '-h' or '-v', we handle them by displaying
 * the usage help or version, respectively. Unexpected will cause a return value of EXIT_FAILURE.
 * While processing the argument array, we also populate a list of pointers to non '-' arguments
 * and return those in the `commandArgs` parameter.
 *
 * @param [in] argc The count of command line arguments in `argv`.
 * @param [in] argv A pointer to the list of command line argument strings.
 * @param [out] commandArgCount A pointer to a int which will contain the number of non '-' arguments in `argv`.
 * @param [out] commandArgs A pointer to an array of pointers. The pointers will be set to the non '-' arguments
 *                          that were passed in in `argv`.
 * @param [out] needToShowUsage A pointer to a boolean that will be set to true if the caller should display the
 *                          usage of this application.
 * @param [out] shouldExit A pointer to a boolean that will be set to true if the caller should exit instead of
 *                         processing the commandArgs.
 *
 * @return EXIT_FAILURE if an unexpected '-' option was encountered. EXIT_SUCCESS otherwise.
 */
int ccnxFileRepoCommon_ProcessCommandLineArguments(int argc, char **argv,
                                                   int *commandArgCount, char **commandArgs,
                                                   bool *needToShowUsage, bool *shouldExit);

#endif // ccnxFileRepoCommon_h
