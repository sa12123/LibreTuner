//
// Created by altenius on 12/20/18.
//

#include "passthru.h"
#include "../network/can/j2534can.h"

namespace lt {
NetworkProtocol passthruToNetworkProtocol(j2534::Protocol passthru) {
    NetworkProtocol protocol{NetworkProtocol::None};
	if ((passthru & j2534::Protocol::CAN) != j2534::Protocol::None) {
		protocol |= NetworkProtocol::Can;
	}
	return protocol;
}

PassThruLink::PassThruLink(j2534::Info &&info)
    : DataLink(info.name), info_(std::move(info)) {}

void PassThruLink::checkInterface() {
    if (!interface_) {
        interface_ = j2534::J2534::create(j2534::Info(info_));
        interface_->init();
    }
}

network::CanPtr PassThruLink::can(uint32_t baudrate) {
    checkDevice();
    return std::make_unique<network::J2534Can>(device_, baudrate);
}

void PassThruLink::checkDevice() {
    if (!device_) {
        checkInterface();
        if (port_.empty())
            device_ = interface_->open();
        else
            device_ = interface_->open(port_.c_str());
        if (!device_) {
            throw std::runtime_error("Failed to create PassThru device.");
        }
    }
}

NetworkProtocol PassThruLink::supportedProtocols() const { return passthruToNetworkProtocol(info_.protocols); }

std::vector<std::unique_ptr<PassThruLink>> detect_passthru_links() {
    std::vector<j2534::Info> info = j2534::detect_interfaces();

    std::vector<std::unique_ptr<PassThruLink>> links;

    for (j2534::Info &i : info) {
        links.emplace_back(
            std::make_unique<PassThruLink>(std::move(i)));
    }

    return links;
}
} // namespace lt
