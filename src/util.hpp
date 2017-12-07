#pragma once
#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtCore/QIODevice>

namespace Saints {

QByteArray decompress_stream(QIODevice& stream, qint64 out_size);

}
