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

#pragma once

#include <QImage>
#include <QVector>
#include <QString>
#include <QByteArray>

/**
 * @class AnimatedQrCode
 * @brief Holds a sequence of QR code frames that together encode a single
 * payload which is too large (or too dense) to display as one QR code.
 *
 * Frame protocol
 * --------------
 * Each frame encodes, in QR binary mode, the following byte sequence:
 *
 *     "{index}/{total}|" + chunk
 *
 * where @c index is the 1-based frame number, @c total is the number of frames
 * in the sequence, and @c chunk is a contiguous slice of the original payload.
 * Concatenating the chunks of frames 1..total (in order) yields the original
 * payload. This is the same format used by
 * https://github.com/tony-xlh/AnimatedQRCodeReader so frames produced by this
 * library can be consumed by that reader (and vice-versa).
 *
 * Example (payload "HelloWorld", chunk size 5):
 *
 *     frame 1: "1/2|Hello"
 *     frame 2: "2/2|World"
 *
 * Use @ref frameDuration() (milliseconds) as the interval of a QTimer to
 * cycle through @ref frame(int) for display.
 */
class AnimatedQrCode
{
public:
  /**
   * @brief Constructs an empty animated QR code (zero frames).
   */
  AnimatedQrCode();

  /**
   * @brief Constructs an animated QR code from a list of pre-rendered frames.
   * @param frames The rendered QR code frames, in sequence order.
   * @param payloads The raw encoded payload of each frame (same order).
   * @param frameDuration The suggested display interval per frame, in ms.
   */
  AnimatedQrCode(const QVector<QImage> &frames,
                 const QVector<QByteArray> &payloads, int frameDuration);

  /**
   * @brief Returns @c true if the sequence contains no frames.
   */
  bool isEmpty() const;

  /**
   * @brief Returns the number of frames in the sequence.
   */
  int frameCount() const;

  /**
   * @brief Returns the suggested display duration of each frame, in
   * milliseconds.
   */
  int frameDuration() const;

  /**
   * @brief Returns the rendered QR code image for the given frame.
   * @param index Zero-based frame index.
   * @return The frame image, or a null QImage if @a index is out of range.
   */
  QImage frame(int index) const;

  /**
   * @brief Returns the raw bytes encoded in the given frame (including the
   * "{index}/{total}|" header). Useful for tests and for custom renderers.
   * @param index Zero-based frame index.
   * @return The frame payload, or an empty QByteArray if out of range.
   */
  QByteArray framePayload(int index) const;

  /**
   * @brief Returns all rendered frames, in sequence order.
   */
  const QVector<QImage> &frames() const;

  /**
   * @brief Encodes all frames as an animated GIF (GIF89a).
   *
   * Frames are rendered with a 2-color palette (black/white) using LZW
   * compression. The per-frame delay is taken from @ref frameDuration()
   * (rounded to 10 ms, the GIF resolution) and the animation loops forever.
   * All frames are scaled to the size of the first frame if they differ.
   *
   * @param loopCount Number of times the animation repeats (0 = infinite).
   * @return The GIF file contents, or an empty QByteArray if there are no
   * frames.
   */
  QByteArray toGif(quint16 loopCount = 0) const;

  /**
   * @brief Convenience wrapper around @ref toGif() that writes the result to
   * @a path.
   * @return @c true on success.
   */
  bool saveGif(const QString &path, quint16 loopCount = 0) const;

  /**
   * @brief Builds the header prepended to a frame's chunk.
   * @param index Zero-based frame index.
   * @param total Total number of frames.
   * @return Header bytes in the form "{index+1}/{total}|".
   */
  static QByteArray frameHeader(int index, int total);

  /**
   * @brief Splits @a data into frame payloads (header + chunk) following the
   * frame protocol described in the class documentation.
   * @param data The payload to split.
   * @param chunkSize Maximum number of payload bytes per frame (must be > 0).
   * @return One QByteArray per frame. An empty payload yields a single frame
   * with an empty chunk ("1/1|").
   */
  static QVector<QByteArray> splitPayload(const QByteArray &data,
                                          int chunkSize);

private:
  QVector<QImage> m_frames;
  QVector<QByteArray> m_payloads;
  int m_frameDuration;
};
