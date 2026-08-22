/*
 * Copyright (c) 2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "AnimatedQrCode.h"

#include <QFile>
#include <QHash>

namespace
{
/**
 * @brief Appends a little-endian 16-bit value to a byte array.
 */
void putU16(QByteArray &out, quint16 value)
{
  out.append(static_cast<char>(value & 0xFF));
  out.append(static_cast<char>((value >> 8) & 0xFF));
}

/**
 * @brief Streams LZW codes into GIF data sub-blocks (max 255 bytes each).
 */
class GifBitWriter
{
public:
  explicit GifBitWriter(QByteArray &out)
    : m_out(out)
    , m_accumulator(0)
    , m_bitCount(0)
  {
  }

  void write(quint32 code, int bits)
  {
    m_accumulator |= code << m_bitCount;
    m_bitCount += bits;
    while (m_bitCount >= 8)
    {
      m_block.append(static_cast<char>(m_accumulator & 0xFF));
      m_accumulator >>= 8;
      m_bitCount -= 8;
      if (m_block.size() == 255)
        flushBlock();
    }
  }

  void finish()
  {
    if (m_bitCount > 0)
      m_block.append(static_cast<char>(m_accumulator & 0xFF));
    m_accumulator = 0;
    m_bitCount = 0;
    flushBlock();
    m_out.append('\0'); // block terminator
  }

private:
  void flushBlock()
  {
    if (m_block.isEmpty())
      return;
    m_out.append(static_cast<char>(m_block.size()));
    m_out.append(m_block);
    m_block.clear();
  }

  QByteArray &m_out;
  QByteArray m_block;
  quint32 m_accumulator;
  int m_bitCount;
};

/**
 * @brief LZW-compresses 1-bit-per-pixel indices into GIF image data.
 * Uses a minimum code size of 2 (smallest allowed by GIF).
 */
void writeLzw(QByteArray &out, const QByteArray &indices)
{
  constexpr int minCodeSize = 2;
  constexpr int clearCode = 1 << minCodeSize; // 4
  constexpr int endCode = clearCode + 1;      // 5
  constexpr int maxCode = 4095;

  out.append(static_cast<char>(minCodeSize));
  GifBitWriter writer(out);

  // Dictionary: key = (prefixCode << 8) | nextByte, value = code
  QHash<quint32, quint16> dict;
  auto resetDict = [&]() {
    dict.clear();
  };

  int codeSize = minCodeSize + 1;
  int nextCode = endCode + 1;
  resetDict();
  writer.write(clearCode, codeSize);

  if (indices.isEmpty())
  {
    writer.write(endCode, codeSize);
    writer.finish();
    return;
  }

  quint32 prefix = static_cast<quint8>(indices[0]);
  for (int i = 1; i < indices.size(); ++i)
  {
    const quint8 byte = static_cast<quint8>(indices[i]);
    const quint32 key = (prefix << 8) | byte;
    auto it = dict.constFind(key);
    if (it != dict.constEnd())
    {
      prefix = it.value();
      continue;
    }

    writer.write(prefix, codeSize);
    if (nextCode <= maxCode)
    {
      dict.insert(key, static_cast<quint16>(nextCode));
      if (nextCode == (1 << codeSize) && codeSize < 12)
        ++codeSize;
      ++nextCode;
    }
    else
    {
      writer.write(clearCode, codeSize);
      resetDict();
      codeSize = minCodeSize + 1;
      nextCode = endCode + 1;
    }

    prefix = byte;
  }

  writer.write(prefix, codeSize);
  writer.write(endCode, codeSize);
  writer.finish();
}
} // namespace

/**
 * @brief Constructs an empty animated QR code.
 */
AnimatedQrCode::AnimatedQrCode()
  : m_frameDuration(0)
{
}

/**
 * @brief Constructs an animated QR code from pre-rendered frames.
 * @param frames Rendered frame images.
 * @param payloads Raw payload of each frame.
 * @param frameDuration Suggested per-frame display interval in milliseconds.
 */
AnimatedQrCode::AnimatedQrCode(const QVector<QImage> &frames,
                               const QVector<QByteArray> &payloads,
                               int frameDuration)
  : m_frames(frames)
  , m_payloads(payloads)
  , m_frameDuration(frameDuration)
{
}

/**
 * @brief Returns true if there are no frames.
 */
bool AnimatedQrCode::isEmpty() const
{
  return m_frames.isEmpty();
}

/**
 * @brief Returns the number of frames.
 */
int AnimatedQrCode::frameCount() const
{
  return m_frames.size();
}

/**
 * @brief Returns the suggested per-frame display interval in milliseconds.
 */
int AnimatedQrCode::frameDuration() const
{
  return m_frameDuration;
}

/**
 * @brief Returns the image for the given frame, or a null image if the index
 * is out of range.
 */
QImage AnimatedQrCode::frame(int index) const
{
  if (index < 0 || index >= m_frames.size())
    return QImage();

  return m_frames.at(index);
}

/**
 * @brief Returns the raw payload of the given frame, or an empty array if the
 * index is out of range.
 */
QByteArray AnimatedQrCode::framePayload(int index) const
{
  if (index < 0 || index >= m_payloads.size())
    return QByteArray();

  return m_payloads.at(index);
}

/**
 * @brief Returns all frames.
 */
const QVector<QImage> &AnimatedQrCode::frames() const
{
  return m_frames;
}

/**
 * @brief Builds the "{index+1}/{total}|" header for a frame.
 */
QByteArray AnimatedQrCode::frameHeader(int index, int total)
{
  return QByteArray::number(index + 1) + '/' + QByteArray::number(total) + '|';
}

/**
 * @brief Splits a payload into frame payloads (header + chunk).
 */
QVector<QByteArray> AnimatedQrCode::splitPayload(const QByteArray &data,
                                                 int chunkSize)
{
  QVector<QByteArray> result;
  if (chunkSize <= 0)
    return result;

  const int total
      = data.isEmpty() ? 1 : (data.size() + chunkSize - 1) / chunkSize;

  result.reserve(total);
  for (int i = 0; i < total; ++i)
  {
    QByteArray payload = frameHeader(i, total);
    payload += data.mid(i * chunkSize, chunkSize);
    result.append(payload);
  }

  return result;
}

/**
 * @brief Encodes the frames as an animated GIF89a with a 2-color palette.
 */
QByteArray AnimatedQrCode::toGif(quint16 loopCount) const
{
  QByteArray out;
  if (m_frames.isEmpty())
    return out;

  const int width = m_frames.first().width();
  const int height = m_frames.first().height();
  if (width <= 0 || height <= 0 || width > 0xFFFF || height > 0xFFFF)
    return out;

  // Header + logical screen descriptor
  out.append("GIF89a");
  putU16(out, static_cast<quint16>(width));
  putU16(out, static_cast<quint16>(height));
  out.append(static_cast<char>(0x80)); // global color table, 2 entries
  out.append('\0');                    // background color index
  out.append('\0');                    // pixel aspect ratio

  // Global color table: index 0 = white, index 1 = black
  out.append("\xFF\xFF\xFF", 3);
  out.append("\x00\x00\x00", 3);

  // NETSCAPE2.0 application extension (looping)
  out.append("\x21\xFF\x0B", 3);
  out.append("NETSCAPE2.0");
  out.append("\x03\x01", 2);
  putU16(out, loopCount);
  out.append('\0');

  // GIF delay resolution is 10 ms
  const quint16 delay
      = static_cast<quint16>(qBound(1, (m_frameDuration + 5) / 10, 0xFFFF));

  for (const QImage &frame : m_frames)
  {
    QImage img = frame;
    if (img.width() != width || img.height() != height)
      img = img.scaled(width, height, Qt::IgnoreAspectRatio,
                       Qt::FastTransformation);
    img = img.convertToFormat(QImage::Format_Grayscale8);

    // Graphic control extension
    out.append("\x21\xF9\x04", 3);
    out.append('\0'); // no disposal, no transparency
    putU16(out, delay);
    out.append('\0'); // transparent color index (unused)
    out.append('\0');

    // Image descriptor
    out.append(',');
    putU16(out, 0);
    putU16(out, 0);
    putU16(out, static_cast<quint16>(width));
    putU16(out, static_cast<quint16>(height));
    out.append('\0'); // no local color table, not interlaced

    // Pixel indices: dark pixel -> 1 (black), otherwise 0 (white)
    QByteArray indices;
    indices.resize(width * height);
    char *dst = indices.data();
    for (int y = 0; y < height; ++y)
    {
      const uchar *line = img.constScanLine(y);
      for (int x = 0; x < width; ++x)
        *dst++ = line[x] < 128 ? 1 : 0;
    }

    writeLzw(out, indices);
  }

  out.append(';'); // trailer
  return out;
}

/**
 * @brief Writes the animated GIF to a file.
 */
bool AnimatedQrCode::saveGif(const QString &path, quint16 loopCount) const
{
  const QByteArray data = toGif(loopCount);
  if (data.isEmpty())
    return false;

  QFile file(path);
  if (!file.open(QFile::WriteOnly | QFile::Truncate))
    return false;

  return file.write(data) == data.size();
}
