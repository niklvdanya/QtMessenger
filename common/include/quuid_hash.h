#pragma once
#include <functional>

#include <QUuid>

struct QUuidHash
{
    std::size_t operator()(const QUuid& uuid) const noexcept { return qHash(uuid); }
};