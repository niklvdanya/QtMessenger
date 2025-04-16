#pragma once
#include <QUuid>

namespace std {
    template<>
    struct hash<QUuid> {
        size_t operator()(const QUuid& uuid) const noexcept {
            return qHash(uuid);
        }
    };
} 