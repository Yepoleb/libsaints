#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QIODevice>

namespace Saints {

QByteArray decompressZLIB(QIODevice& stream);
QByteArray decompressLZ4(QIODevice& stream);

}
