#pragma once
#include <QUuid>
#include <functional>

struct QUuidHash {
    std::size_t operator()(const QUuid& uuid) const noexcept {
        return qHash(uuid);
    }
};