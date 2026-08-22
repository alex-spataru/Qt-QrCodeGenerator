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

#include "MainWindow.h"

#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>

/**
 * Constructor function, initializes the user interface & setups signals/slots.
 */
MainWindow::MainWindow()
  : m_gifBuffer(nullptr)
  , m_movie(nullptr)
{
  // Initialize layout-related widgets
  m_centralWidget = new QWidget(this);
  m_header = new QWidget(m_centralWidget);

  // Initialize horizontal layout widgets
  m_label = new QLabel(tr("Message") + ": ", m_header);
  m_saveButton = new QPushButton(tr("Export QR Code"), m_header);
  m_lineEdit = new QLineEdit(m_header);
  m_animatedCheck = new QCheckBox(tr("Animated"), m_header);
  m_chunkSizeSpin = new QSpinBox(m_header);
  m_chunkSizeSpin->setRange(1, 2000);
  m_chunkSizeSpin->setValue(16);
  m_chunkSizeSpin->setPrefix(tr("Chunk") + ": ");
  m_chunkSizeSpin->setEnabled(false);

  // Initialize QR code image label
  m_qrCodeDisplay = new QLabel(m_centralWidget);
  m_qrCodeDisplay->setMinimumSize(QSize(500, 500));
  m_qrCodeDisplay->setMaximumSize(QSize(500, 500));

  // Setup horizontal layout for line edit & push button
  m_verticalLayout = new QVBoxLayout(m_centralWidget);
  m_headerLayout = new QHBoxLayout(m_header);
  m_headerLayout->addWidget(m_label);
  m_headerLayout->addWidget(m_lineEdit);
  m_headerLayout->addWidget(m_animatedCheck);
  m_headerLayout->addWidget(m_chunkSizeSpin);
  m_headerLayout->addWidget(m_saveButton);
  m_headerLayout->setContentsMargins(0, 0, 0, 0);
  m_headerLayout->setStretch(1, 1);

  // Setup vertical layout with QR code label & QR code generation controls
  m_verticalLayout->addWidget(m_header);
  m_verticalLayout->addWidget(m_qrCodeDisplay);
  m_verticalLayout->setStretch(0, 1);

  // Set central widget & resize window to minimum size possible
  setCentralWidget(m_centralWidget);
  setFixedSize(minimumSize());

  // Set window title
  setWindowTitle(tr("QR Code Generator"));

  // Update the QR code when the text changes
  connect(m_lineEdit, &QLineEdit::textChanged, this,
          [this](const QString &text) {
            m_saveButton->setEnabled(!text.isEmpty());
            if (!text.isEmpty())
              generateQrCode();
          });

  // Regenerate when animation settings change
  connect(m_animatedCheck, &QCheckBox::toggled, this, [this](bool checked) {
    m_chunkSizeSpin->setEnabled(checked);
    generateQrCode();
  });
  connect(m_chunkSizeSpin, &QSpinBox::valueChanged, this,
          [this](int) { generateQrCode(); });

  // Export QR code as when user clicks on the save button
  connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::saveQrCode);

  // Set "Hello World" text & generate QR code
  m_lineEdit->setText(tr("Hello World!"));
  generateQrCode();
}

/**
 * Generates a QR code from text input & saves it as a SVG file (or an
 * animated GIF when animated mode is enabled).
 */
void MainWindow::saveQrCode()
{
  // Animated mode: export the animated GIF currently being displayed
  if (m_animatedCheck->isChecked() && !m_gifData.isEmpty())
  {
    auto path = QFileDialog::getSaveFileName(this, tr("Save Animated QR Code"),
                                             QDir::homePath(), tr("*.gif"));
    if (!path.isEmpty())
    {
      QFile file(path);
      if (file.open(QFile::WriteOnly))
      {
        file.write(m_gifData);
        file.close();
      }
    }

    return;
  }

  // Obtain SVG data for QR code
  auto svg = m_generator.generateSvgQr(m_lineEdit->text());
  if (svg.isEmpty())
    return;

  // Ask user where to save the SVG file
  auto path = QFileDialog::getSaveFileName(this, tr("Save QR Code"),
                                           QDir::homePath(), tr("*.svg"));

  // Write SVG data into the file selected by the user
  if (!path.isEmpty())
  {
    QFile file(path);
    if (file.open(QFile::WriteOnly))
    {
      file.write(svg.toUtf8());
      file.close();
    }
  }
}

/**
 * Generates a QR code from text input & displays it in the UI.
 */
void MainWindow::generateQrCode()
{
  // Get text from line edit & target display size
  auto text = m_lineEdit->text();
  const auto displaySize = m_qrCodeDisplay->size();
  const quint16 qrSize = static_cast<quint16>(
      qMin(displaySize.width(), displaySize.height()));

  // Tear down any previous animation
  if (m_movie)
  {
    m_qrCodeDisplay->setMovie(nullptr);
    delete m_movie;
    delete m_gifBuffer;
    m_movie = nullptr;
    m_gifBuffer = nullptr;
  }

  // Animated mode: generate a looping GIF & let QMovie handle playback
  if (m_animatedCheck->isChecked())
  {
    m_gifData = m_generator.generateGifQr(text, m_chunkSizeSpin->value(), 500,
                                          qrSize);

    m_gifBuffer = new QBuffer(this);
    m_gifBuffer->setData(m_gifData);
    m_gifBuffer->open(QIODevice::ReadOnly);

    m_movie = new QMovie(m_gifBuffer, "gif", this);
    m_movie->setScaledSize(displaySize);
    m_qrCodeDisplay->setMovie(m_movie);
    m_qrCodeDisplay->setToolTip(text);
    m_movie->start();
    return;
  }

  // Generate QR code from text
  QImage unscaledImage = m_generator.generateQr(text, qrSize);
  QImage image = unscaledImage.scaled(displaySize);

  // Display generated image
  m_qrCodeDisplay->setPixmap(QPixmap::fromImage(image));
  m_qrCodeDisplay->setToolTip(text);
}

