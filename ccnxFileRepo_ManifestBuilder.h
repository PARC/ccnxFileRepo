/*
 * Copyright (c) 2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
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
 * @file magic_Foobar.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 * @author <#gscott#>, Computing Science Laboratory, PARC
 * @copyright (c) 2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#ifndef libccnx_common_ccnx_ManifestBuilder
#define libccnx_common_ccnx_ManifestBuilder

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

struct ccnx_manifest_builder;
typedef struct ccnx_manifest_builder CCNxManifestBuilder;

/**
 * Increase the number of references to a `CCNxManifestBuilder` instance.
 *
 * Note that new `CCNxManifestBuilder` is not created,
 * only that the given `CCNxManifestBuilder` reference count is incremented.
 * Discard the reference by invoking `ccnxManifestBuilder_Release`.
 *
 * @param [in] instance A pointer to a valid CCNxManifestBuilder instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     CCNxManifestBuilder *b = ccnxManifestBuilder_Acquire();

 *     ccnxManifestBuilder_Release(&a);
 *     ccnxManifestBuilder_Release(&b);
 * }
 * @endcode
 */
CCNxManifestBuilder *ccnxManifestBuilder_Acquire(const CCNxManifestBuilder *instance);

#ifdef CCNxManifestBuilder_DISABLE_VALIDATION
#  define ccnxManifestBuilder_OptionalAssertValid(_instance_)
#else
#  define ccnxManifestBuilder_OptionalAssertValid(_instance_) ccnxManifestBuilder_AssertValid(_instance_)
#endif

/**
 * Assert that the given `CCNxManifestBuilder` instance is valid.
 *
 * @param [in] instance A pointer to a valid CCNxManifestBuilder instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     ccnxManifestBuilder_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     ccnxManifestBuilder_Release(&b);
 * }
 * @endcode
 */
void ccnxManifestBuilder_AssertValid(const CCNxManifestBuilder *instance);

/**
 * Create an instance of CCNxManifestBuilder
 *
 * @return non-NULL A pointer to a valid CCNxManifestBuilder instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     ccnxManifestBuilder_Release(&a);
 * }
 * @endcode
 */
CCNxManifestBuilder *ccnxManifestBuilder_Create();

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid CCNxManifestBuilder instance.
 * @param [in] other A pointer to a valid CCNxManifestBuilder instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *     CCNxManifestBuilder *b = ccnxManifestBuilder_Create();
 *
 *     if (ccnxManifestBuilder_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     ccnxManifestBuilder_Release(&a);
 *     ccnxManifestBuilder_Release(&b);
 * }
 * @endcode
 *
 * @see ccnxManifestBuilder_Equals
 */
int ccnxManifestBuilder_Compare(const CCNxManifestBuilder *instance, const CCNxManifestBuilder *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid CCNxManifestBuilder instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `CCNxManifestBuilder` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     CCNxManifestBuilder *copy = ccnxManifestBuilder_Copy(&b);
 *
 *     ccnxManifestBuilder_Release(&b);
 *     ccnxManifestBuilder_Release(&copy);
 * }
 * @endcode
 */
CCNxManifestBuilder *ccnxManifestBuilder_Copy(const CCNxManifestBuilder *original);

/**
 * Print a human readable representation of the given `CCNxManifestBuilder`.
 *
 * @param [in] instance A pointer to a valid CCNxManifestBuilder instance.
 * @param [in] indentation The indentation level to use for printing.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     ccnxManifestBuilder_Display(a, 0);
 *
 *     ccnxManifestBuilder_Release(&a);
 * }
 * @endcode
 */
void ccnxManifestBuilder_Display(const CCNxManifestBuilder *instance, int indentation);

/**
 * Determine if two `CCNxManifestBuilder` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxManifestBuilder` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `ccnxManifestBuilder_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxManifestBuilder_Equals(x, y)` must return true if and only if
 *        `ccnxManifestBuilder_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxManifestBuilder_Equals(x, y)` returns true and
 *        `ccnxManifestBuilder_Equals(y, z)` returns true,
 *        then `ccnxManifestBuilder_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxManifestBuilder_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxManifestBuilder_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid CCNxManifestBuilder instance.
 * @param [in] y A pointer to a valid CCNxManifestBuilder instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *     CCNxManifestBuilder *b = ccnxManifestBuilder_Create();
 *
 *     if (ccnxManifestBuilder_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     ccnxManifestBuilder_Release(&a);
 *     ccnxManifestBuilder_Release(&b);
 * }
 * @endcode
 * @see ccnxManifestBuilder_HashCode
 */
bool ccnxManifestBuilder_Equals(const CCNxManifestBuilder *x, const CCNxManifestBuilder *y);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of `HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the `HashCode` function must consistently return the same value,
 * provided no information used in a corresponding comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the {@link ccnxManifestBuilder_Equals} method,
 * then calling the {@link ccnxManifestBuilder_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link ccnxManifestBuilder_Equals} function,
 * then calling the `ccnxManifestBuilder_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid CCNxManifestBuilder instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     PARCHashCode hashValue = ccnxManifestBuilder_HashCode(buffer);
 *     ccnxManifestBuilder_Release(&a);
 * }
 * @endcode
 */
PARCHashCode ccnxManifestBuilder_HashCode(const CCNxManifestBuilder *instance);

/**
 * Determine if an instance of `CCNxManifestBuilder` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid CCNxManifestBuilder instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     if (ccnxManifestBuilder_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     ccnxManifestBuilder_Release(&a);
 * }
 * @endcode
 *
 */
bool ccnxManifestBuilder_IsValid(const CCNxManifestBuilder *instance);

/**
 * Release a previously acquired reference to the given `CCNxManifestBuilder` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     ccnxManifestBuilder_Release(&a);
 * }
 * @endcode
 */
void ccnxManifestBuilder_Release(CCNxManifestBuilder **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid CCNxManifestBuilder instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     PARCJSON *json = ccnxManifestBuilder_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     ccnxManifestBuilder_Release(&a);
 * }
 * @endcode
 */
PARCJSON *ccnxManifestBuilder_ToJSON(const CCNxManifestBuilder *instance);

/**
 * Produce a null-terminated string representation of the specified `CCNxManifestBuilder`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid CCNxManifestBuilder instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *a = ccnxManifestBuilder_Create();
 *
 *     char *string = ccnxManifestBuilder_ToString(a);
 *
 *     ccnxManifestBuilder_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see ccnxManifestBuilder_Display
 */
char *ccnxManifestBuilder_ToString(const CCNxManifestBuilder *instance);

/**
 * Produce a skewed Manifest.
 *
 * Each HashGroup in a manifest will contain a list of data pointers and then
 * a single Manifest pointer.
 *
 * @param [in] instance The `CCNxManifestBuilder`.
 * @param [in] chunker A `PARCChunker` that chunks up the data to be created.
 * @param [in] name The `CCNxName` of the Manifest. This may not be null.
 *
 * Example:
 * @code
 * {
 *     CCNxManifestBuilder *builder = ccnxManifestBuilder_Create();
 *     PARCChunker *chunker = ...
 *     CCNxName *manifestName = ccnxName_CreateFromCString("ccnx:/my/manifest");
 *
 *     CCNxManifest *manifest = ccnxManifestBuilder_BuildSkewedManifest(builder, chunker, manifestName);
 *
 *     ccnxManifestBuilder_Release(&builder);
 *     parcChunker_Release(&chunker);
 *     ccnxName_Release(&manifestName);
 * }
 * @endcode
 */
PARCLinkedList *ccnxManifestBuilder_BuildSkewedManifest(const CCNxManifestBuilder *instance, PARCChunker *chunker, const CCNxName *name);
#endif // libccnx_common_ccnx_ManifestBuilder
