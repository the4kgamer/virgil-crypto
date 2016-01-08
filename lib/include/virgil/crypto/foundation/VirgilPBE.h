/**
 * Copyright (C) 2015 Virgil Security Inc.
 *
 * Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3) Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef VIRGIL_CRYPTO_VIRGIL_PBE_H
#define VIRGIL_CRYPTO_VIRGIL_PBE_H

#include <cstddef>

#include <virgil/crypto/VirgilByteArray.h>
#include <virgil/crypto/foundation/asn1/VirgilAsn1Compatible.h>

namespace virgil { namespace crypto { namespace foundation {

/**
 * @name Forward declarations
 */
///@{
class VirgilPBEImpl;
///@}

/**
 * @brief Provides Password-Based Cryptography. Now PKCS#5 and PKCS#12 are partially supported.
 */
class VirgilPBE : public virgil::crypto::foundation::asn1::VirgilAsn1Compatible {
public:
    /**
     * @name Constants
     */
    ///@{
    enum {
        kIterationCountMin = 1024
    };
    ///@}
public:
    /**
     * @name Creation methods
     * @brief Object creation with specific hash function.
     */
    ///@{
    /**
     * @brief Create object with PKCS#5 parameters for PBE encryption or decryption.
     * @note Recommended PKCS#5 parameters are set.
     */
    static VirgilPBE pkcs5(const virgil::crypto::VirgilByteArray& salt, size_t iterationCount = kIterationCountMin);
    /**
     * @brief Create object with PKCS#12 parameters for PBE encryption or decryption.
     * @note Recommended PKCS#12 parameters are set.
     */
    static VirgilPBE pkcs12(const virgil::crypto::VirgilByteArray& salt, size_t iterationCount = kIterationCountMin);
    ///@}
    /**
     * @name Constructor / Destructor
     */
    ///@{
    /**
     * @brief Create object with undefined algorithm.
     * @warning SHOULD be used in conjunction with VirgilAsn1Compatible interface,
     *     i.e. VirgilPBE pbe = VirgilPBE().fromAsn1(asn1);
     */
    VirgilPBE();
    /**
     * @brief Polymorphic destructor.
     */
    virtual ~VirgilPBE() throw();
    ///@}
    /**
     * @name Encryption / Decryption
     */
    ///@{
    /**
     * @brief Encrypt data with given password.
     * @param data - data to encrypt.
     * @param pwd - password to use for encryption (max length is 31 byte).
     * @return Encrypted data.
     */
    virgil::crypto::VirgilByteArray encrypt(const virgil::crypto::VirgilByteArray& data,
            const virgil::crypto::VirgilByteArray& pwd) const;
    /**
     * @brief Decrypt data with given password.
     * @param data - data to decrypt.
     * @param pwd - password to use for decryption (max length is 31 byte).
     * @return Decrypted data.
     */
    virgil::crypto::VirgilByteArray decrypt(const virgil::crypto::VirgilByteArray& data,
            const virgil::crypto::VirgilByteArray& pwd) const;
    ///@}
    /**
     * @name Copy constructor / assignment operator
     * @warning Copy constructor and assignment operator create copy of the object as it was created
     *          by on of the creation methods. All changes in the internal state,
     *          that was made after creation, are not copied!
     */
    ///@{
    VirgilPBE(const VirgilPBE& other);
    VirgilPBE& operator=(const VirgilPBE& rhs);
    ///@}
    /**
     * @name VirgilAsn1Compatible implementation
     * @code
     * Marshalling format:
     *     PBE ::= AlgorithmIdentifier {{ PBEAlgorithms }}
     *     PBEAlgorithms AlgorithmIdentifier ::= {
     *         { OID id-PBES2 PARMS PBES2-params }  |
     *         { OID pkcs-12PbeId PARMS pkcs-12PbeParams }
     *     }
     * @endcode
     */
    ///@{
    virtual size_t asn1Write(virgil::crypto::foundation::asn1::VirgilAsn1Writer& asn1Writer,
            size_t childWrittenBytes = 0) const;
    virtual void asn1Read(virgil::crypto::foundation::asn1::VirgilAsn1Reader& asn1Reader);
    ///@}
private:
    /**
     * @brief Creates and initialize PBKDF with specified type.
     * @warning Constructor CAN NOT be used directly, use one of factory methods to create apropriate cipher.
     */
    explicit VirgilPBE(int type, const virgil::crypto::VirgilByteArray& salt, size_t iterationCount);
    /**
     * @brief If internal state is not initialized with specific algorithm exception will be thrown.
     */
    void checkState() const;
    /**
     * @brief Encrypt or decrypt data depend on the mode.
     */
    virgil::crypto::VirgilByteArray process(const virgil::crypto::VirgilByteArray& data,
            const virgil::crypto::VirgilByteArray& pwd, int mode) const;
private:
    VirgilPBEImpl *impl_;
};

}}}

#endif /* VIRGIL_CRYPTO_VIRGIL_PBE_H */
