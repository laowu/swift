/*
 * Copyright (c) 2010-2016 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include <Swiften/Chat/ChatStateNotifier.h>

#include <memory>

#include <boost/bind.hpp>

#include <Swiften/Client/StanzaChannel.h>
#include <Swiften/Disco/EntityCapsProvider.h>
#include <Swiften/Elements/ChatState.h>
#include <Swiften/Elements/Message.h>

namespace Swift {

ChatStateNotifier::ChatStateNotifier(StanzaChannel* stanzaChannel, const JID& contact, EntityCapsProvider* entityCapsManager) : stanzaChannel_(stanzaChannel), entityCapsManager_(entityCapsManager), contact_(contact) {
    setContact(contact);
    entityCapsManager_->onCapsChanged.connect(boost::bind(&ChatStateNotifier::handleCapsChanged, this, _1));
}

ChatStateNotifier::~ChatStateNotifier() {
    entityCapsManager_->onCapsChanged.disconnect(boost::bind(&ChatStateNotifier::handleCapsChanged, this, _1));
}

void ChatStateNotifier::setContact(const JID& contact) {
    contactHasSentActive_ = false;
    userIsTyping_ = false;
    contactIsOnline_ = false;
    contact_ = contact;
    handleCapsChanged(contact_);
}

void ChatStateNotifier::setContactIsOnline(bool online) {
    contactIsOnline_ = online;
}

void ChatStateNotifier::setUserIsTyping() {
    bool should = contactShouldReceiveStates();
    if (should && !userIsTyping_) {
        userIsTyping_ = true;
        changeState(ChatState::Composing);
    }
}

void ChatStateNotifier::userSentMessage() {
    userIsTyping_ = false;
}

void ChatStateNotifier::userCancelledNewMessage() {
    if (userIsTyping_) {
        userIsTyping_ = false;
        changeState(ChatState::Active);
    }
}

void ChatStateNotifier::receivedMessageFromContact(bool hasActiveElement) {
    contactHasSentActive_ = hasActiveElement;
}

bool ChatStateNotifier::contactShouldReceiveStates() {
    /* So, yes, the XEP says to look at caps, but it also says that once you've
         heard from the contact, the active state overrides this.
         *HOWEVER* it says that the MUST NOT send csn if you haven't received
         active is OPTIONAL behaviour for if you haven't got caps.*/
    return contactIsOnline_ && (contactHasSentActive_ || contactHas85Caps_);
}

void ChatStateNotifier::changeState(ChatState::ChatStateType state) {
    std::shared_ptr<Message> message(std::make_shared<Message>());
    message->setTo(contact_);
    message->addPayload(std::make_shared<ChatState>(state));
    stanzaChannel_->sendMessage(message);
}

void ChatStateNotifier::addChatStateRequest(Message::ref message) {
    if (contactShouldReceiveStates()) {
        message->addPayload(std::make_shared<ChatState>(ChatState::Active));
    }
}


void ChatStateNotifier::handleCapsChanged(const JID& jid) {
    if (jid == contact_) {
        DiscoInfo::ref caps = entityCapsManager_->getCaps(contact_);
        bool hasCSN = caps && caps->hasFeature(DiscoInfo::ChatStatesFeature);
        contactHas85Caps_ = hasCSN;
    }
}

}
