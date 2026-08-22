# Qt QR Code Generator Library

Qt QR Code Generator is a simple C++ class that uses the [qrcodegen](https://github.com/nayuki/QR-Code-generator) library to generate QR codes from QStrings in Qt applications. It can also
split large payloads into [animated QR codes](#animated-qr-codes) and export
them as looping GIFs.

[![Demo](example/demo.gif)](example/demo.gif)

## Usage

1. Copy the *Qt-QrCodeGenerator* folder in your `lib` folder.
2. Include the *Qt-QrCodeGenerator* project include (pri) file using the `include()` qmake function.
3. Use the `QrCodeGenerator` class in your code:

```cpp
#include <QrCodeGenerator>

QrCodeGenerator generator;
QString data = "https://www.example.com";
QImage qrCodeImage = generator.generateQr(data);
```

4. That's all! Check the [example](example) project as a reference for your project if needed.

Here are the steps to run the example using cmake (specific steps for LINUX ONLY):
```sh
cd ./example
mkdir -p build
cd build
cmake ../
make
./qrexample # runs executable
```

### Animated QR codes

For payloads too large to fit (or scan reliably) in a single QR code, the
library can split the data across several frames that are displayed in
sequence. The simplest way is to ask for an animated GIF and let `QMovie`
handle playback:

```cpp
#include <QrCodeGenerator>
#include <QBuffer>
#include <QMovie>

QrCodeGenerator generator;
QByteArray data = ...; // large payload, QString also supported

// 512 bytes per frame, 250 ms per frame -> looping GIF89a
QByteArray gif = generator.generateGifQr(data, 512, 250);

auto *buffer = new QBuffer(this);
buffer->setData(gif);
buffer->open(QIODevice::ReadOnly);
auto *movie = new QMovie(buffer, "gif", this);
label->setMovie(movie);
movie->start();

// ...or save it to disk
QFile file("qr.gif");
file.open(QFile::WriteOnly);
file.write(gif);
```

If you need access to the individual frames (custom rendering, your own
timer, etc.), use `generateAnimatedQr()` instead:

```cpp
AnimatedQrCode animated = generator.generateAnimatedQr(data, 512, 250);

int count = animated.frameCount();   // number of frames
QImage frame = animated.frame(0);    // frame images, all the same size
int delay = animated.frameDuration(); // suggested ms per frame
animated.saveGif("qr.gif");          // same GIF encoder as generateGifQr()
```

#### Frame protocol

Each frame is a QR code (binary mode) containing:

```
"{index}/{total}|" + chunk
```

* `index` is the 1-based frame number, `total` the number of frames.
* `chunk` is a contiguous slice of the original payload, at most `chunkSize`
  bytes long. Concatenating the chunks of frames `1..total` yields the original
  payload.

This is the same format used by
[AnimatedQRCodeReader](https://github.com/tony-xlh/AnimatedQRCodeReader), so
frames produced by this library can be decoded by that reader. Error detection
within each frame is provided by the QR code's own error correction; the header
lets a reader detect missing or duplicated frames and reassemble out of order.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
