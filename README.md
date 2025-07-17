# Qt QR Code Generator Library

Qt QR Code Generator is a simple C++ class that uses the [qrcodegen](https://github.com/nayuki/QR-Code-generator) library to generate QR codes from QStrings in Qt applications.

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

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
