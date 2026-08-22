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
#include <QObject>
#include <QString>

#include "qrcodegen/qrcodegen.h"
#include "AnimatedQrCode.h"

/**
 * @class QrCodeGenerator
 * @brief The QrCodeGenerator class is a simple C++ class that uses the
 * qrcodegen library to generate QR codes from QStrings in Qt applications.
 */
class QrCodeGenerator : public QObject
{
public:
  /**
   * @brief Constructs a QrCodeGenerator object.
   * @param parent The parent QObject.
   */
  explicit QrCodeGenerator(QObject *parent = nullptr);

  /**
   * @brief Generates a QR code from the given data and error correction level.
   * @param data The QString containing the data to encode in the QR code.
   * @param size The desired width/height of the generated image (default: 500).
   * @param borderSize The desired border width of the generated image (default:
   * 1).
   * @param errorCorrection The desired error correction level (default:
   * qrcodegen::QrCode::Ecc::MEDIUM).
   *
   * @return QImage containing the generated QR code.
   */
  QImage generateQr(const QString &data, const quint16 size = 1000,
                    const quint16 borderSize = 1,
                    qrcodegen::QrCode::Ecc errorCorrection
                    = qrcodegen::QrCode::Ecc::MEDIUM);

  /**
   * @brief Generates a QR code from the given data and error correction level.
   * @param data The QByteArray containing the data to encode in the QR code.
   * @param size The desired width/height of the generated image (default: 500).
   * @param borderSize The desired border width of the generated image (default:
   * 1).
   * @param errorCorrection The desired error correction level (default:
   * qrcodegen::QrCode::Ecc::MEDIUM).
   *
   * @return QImage containing the generated QR code.
   */
  QImage generateQr(const QByteArray &data, const quint16 size = 1000,
                    const quint16 borderSize = 1,
                    qrcodegen::QrCode::Ecc errorCorrection
                    = qrcodegen::QrCode::Ecc::MEDIUM);

  /**
   * @brief Generates a QR code from the given data and error correction level.
   * @param data The QString containing the data to encode in the QR code.
   * @param borderSize The desired border width of the generated image (default:
   * 1).
   * @param errorCorrection The desired error correction level (default:
   * qrcodegen::QrCode::Ecc::MEDIUM).
   *
   * @return QString string containing the generated QR code in SVG format.
   */
  QString generateSvgQr(const QString &data, const quint16 borderSize = 1,
                        qrcodegen::QrCode::Ecc errorCorrection
                        = qrcodegen::QrCode::Ecc::MEDIUM);

  /**
   * @brief Generates an animated QR code (a sequence of QR frames) from the
   * given binary payload.
   *
   * The payload is split into chunks of at most @a chunkSize bytes. Each chunk
   * is prefixed with a "{index}/{total}|" header and encoded as its own QR
   * code frame. See @ref AnimatedQrCode for the full frame protocol.
   *
   * @param data The QByteArray containing the data to encode.
   * @param chunkSize Maximum payload bytes per frame (default: 512). Values
   * above what the chosen error-correction level can encode in a single QR
   * code will throw qrcodegen::data_too_long.
   * @param frameDuration Suggested display interval per frame, in
   * milliseconds (default: 250).
   * @param size The desired width/height of each frame image (default: 1000).
   * @param borderSize The desired border width of each frame (default: 1).
   * @param errorCorrection The desired error correction level (default:
   * qrcodegen::QrCode::Ecc::MEDIUM).
   *
   * @return AnimatedQrCode containing the generated frames.
   */
  AnimatedQrCode generateAnimatedQr(const QByteArray &data,
                                    const int chunkSize = 512,
                                    const int frameDuration = 250,
                                    const quint16 size = 1000,
                                    const quint16 borderSize = 1,
                                    qrcodegen::QrCode::Ecc errorCorrection
                                    = qrcodegen::QrCode::Ecc::MEDIUM);

  /**
   * @brief Generates an animated QR code (a sequence of QR frames) from the
   * given text. The text is encoded as UTF-8 before being split into frames.
   *
   * @param data The QString containing the data to encode.
   * @param chunkSize Maximum payload bytes per frame (default: 512).
   * @param frameDuration Suggested display interval per frame, in
   * milliseconds (default: 250).
   * @param size The desired width/height of each frame image (default: 1000).
   * @param borderSize The desired border width of each frame (default: 1).
   * @param errorCorrection The desired error correction level (default:
   * qrcodegen::QrCode::Ecc::MEDIUM).
   *
   * @return AnimatedQrCode containing the generated frames.
   */
  AnimatedQrCode generateAnimatedQr(const QString &data,
                                    const int chunkSize = 512,
                                    const int frameDuration = 250,
                                    const quint16 size = 1000,
                                    const quint16 borderSize = 1,
                                    qrcodegen::QrCode::Ecc errorCorrection
                                    = qrcodegen::QrCode::Ecc::MEDIUM);

  /**
   * @brief Generates an animated QR code and encodes it directly as an
   * animated GIF (GIF89a) that loops forever.
   *
   * This is a convenience wrapper around @ref generateAnimatedQr() and
   * AnimatedQrCode::toGif(). The result can be written to a .gif file, or
   * displayed without any timer logic using QMovie:
   *
   * @code
   * auto *buffer = new QBuffer(this);
   * buffer->setData(generator.generateGifQr(data));
   * buffer->open(QIODevice::ReadOnly);
   * auto *movie = new QMovie(buffer, "gif", this);
   * label->setMovie(movie);
   * movie->start();
   * @endcode
   *
   * @param data The QByteArray containing the data to encode.
   * @param chunkSize Maximum payload bytes per frame (default: 512).
   * @param frameDuration Display interval per frame, in milliseconds
   * (default: 250, rounded to 10 ms by the GIF format).
   * @param size The desired width/height of each frame image (default: 1000).
   * @param borderSize The desired border width of each frame (default: 1).
   * @param errorCorrection The desired error correction level (default:
   * qrcodegen::QrCode::Ecc::MEDIUM).
   *
   * @return QByteArray with the GIF file contents.
   */
  QByteArray generateGifQr(const QByteArray &data, const int chunkSize = 512,
                           const int frameDuration = 250,
                           const quint16 size = 1000,
                           const quint16 borderSize = 1,
                           qrcodegen::QrCode::Ecc errorCorrection
                           = qrcodegen::QrCode::Ecc::MEDIUM);

  /**
   * @brief Generates an animated QR code from text and encodes it directly as
   * an animated GIF. See the QByteArray overload for details.
   */
  QByteArray generateGifQr(const QString &data, const int chunkSize = 512,
                           const int frameDuration = 250,
                           const quint16 size = 1000,
                           const quint16 borderSize = 1,
                           qrcodegen::QrCode::Ecc errorCorrection
                           = qrcodegen::QrCode::Ecc::MEDIUM);

private:
  /**
   * @brief Converts a qrcodegen::QrCode object to a SVG image.
   * @param qrCode The qrcodegen::QrCode object to convert.
   * @param border The desired border width of the generated image (default: 1).
   *
   * @return SVG containing the QR code.
   */
  QString toSvgString(const qrcodegen::QrCode &qr, quint16 border) const;

  /**
   * @brief Converts a qrcodegen::QrCode object to a QImage.
   * @param qrCode The qrcodegen::QrCode object to convert.
   * @param size The desired width/height of the generated image.
   * @param borderSize The desired border width of the generated image.
   *
   * @return QImage containing the QR code.
   */
  QImage qrCodeToImage(const qrcodegen::QrCode &qrCode, quint16 border,
                       const quint16 size) const;
};
