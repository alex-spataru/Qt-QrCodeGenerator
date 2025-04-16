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
{
  // Initialize layout-related widgets
  m_centralWidget = new QWidget(this);
  m_header = new QWidget(m_centralWidget);

  // Initialize horizontal layout widgets
  m_label = new QLabel(tr("Message") + ": ", m_header);
  m_saveButton = new QPushButton(tr("Export QR Code"), m_header);
  m_lineEdit = new QLineEdit(m_header);

  // Initialize QR code image label
  m_qrCodeDisplay = new QLabel(m_centralWidget);
  m_qrCodeDisplay->setMinimumSize(QSize(500, 500));
  m_qrCodeDisplay->setMaximumSize(QSize(500, 500));

  // Setup horizontal layout for line edit & push button
  m_verticalLayout = new QVBoxLayout(m_centralWidget);
  m_headerLayout = new QHBoxLayout(m_header);
  m_headerLayout->addWidget(m_label);
  m_headerLayout->addWidget(m_lineEdit);
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

  // Export QR code as when user clicks on the save button
  connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::saveQrCode);

  // Set "Hello World" text & generate QR code
  m_lineEdit->setText(tr("Hello World!"));
  generateQrCode();
}

/**
 * Destructor function.
 */
MainWindow::~MainWindow()
{
  delete m_label;
  delete m_lineEdit;
  delete m_saveButton;
  delete m_qrCodeDisplay;
  delete m_headerLayout;
  delete m_verticalLayout;
  delete m_header;
  delete m_centralWidget;
}

/**
 * Generates a QR code from text input & saves it as a SVG file.
 */
void MainWindow::saveQrCode()
{
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
  // Get text from line edit
  auto text = m_lineEdit->text();

  // Generate QR code from text
  QImage unscaledImage = m_generator.generateQr(text);
  QImage image = unscaledImage.scaled(500, 500);

  // Display generated image
  m_qrCodeDisplay->setPixmap(QPixmap::fromImage(image));
  m_qrCodeDisplay->setToolTip(text);
}
