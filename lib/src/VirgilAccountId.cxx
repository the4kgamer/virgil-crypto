/**
 * Copyright (C) 2014 Virgil Security Inc.
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

#include "virgil/service/data/VirgilAccountId.h"
using virgil::service::data::VirgilAccountId;

#include <virgil/VirgilByteArray.h>
using virgil::VirgilByteArray;

#include <virgil/crypto/asn1/VirgilAsn1Reader.h>
using virgil::crypto::asn1::VirgilAsn1Reader;

#include <virgil/crypto/asn1/VirgilAsn1Writer.h>
using virgil::crypto::asn1::VirgilAsn1Writer;

#include <json/json.h>

/**
 * @name JSON Keys
 */
///@{
static const char *kJsonKey_AccountId = "account_id";
///@}

VirgilByteArray VirgilAccountId::accountId() const {
    return accountId_;
}

void VirgilAccountId::setAccountId(const VirgilByteArray& accountId) {
    accountId_ = accountId;
}

size_t VirgilAccountId::asn1Write(VirgilAsn1Writer& asn1Writer, size_t childWrittenBytes) const {
    size_t writtenBytes = asn1Writer.writeUTF8String(accountId_);
    return VirgilId::asn1Write(asn1Writer, writtenBytes + childWrittenBytes);
}

bool VirgilAccountId::isEmpty() const {
    return accountId_.empty();
}

void VirgilAccountId::clear() {
    accountId_.clear();
}

void VirgilAccountId::asn1Read(VirgilAsn1Reader& asn1Reader) {
    VirgilId::asn1Read(asn1Reader);
    accountId_ = asn1Reader.readUTF8String();
}

Json::Value VirgilAccountId::jsonWrite(Json::Value& childValue) const {
    childValue[kJsonKey_AccountId] = virgil::bytes2str(accountId_);
    return VirgilId::jsonWrite(childValue);
}

Json::Value VirgilAccountId::jsonRead(const Json::Value& parentValue) {
    Json::Value childValue = VirgilId::jsonRead(parentValue);
    accountId_ = jsonGetStringAsByteArray(childValue, kJsonKey_AccountId);
    return childValue;
}
