#include "mnist/loader.h"

#include "mnist/exception.h"

#include <array>
#include <fstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

std::array<MNist::Loader::DataPair, 2> MNist::Loader::loadData() const {
  auto train{loadImages(m_trainingLabelsPath, m_trainingImagesPath)};
  auto test{loadImages(m_testingLabelsPath, m_testingImagesPath)};

  return std::array{train, test};
}

MNist::Loader::DataPair
MNist::Loader::loadImages(const std::string &labelsPath,
                          const std::string &imagesPath) {
  std::vector<double> labels{loadLabelsFile(labelsPath)};
  std::vector<Math::Matrix<double>> images{loadImagesFile(imagesPath)};

  return std::make_tuple(std::move(labels), std::move(images));
}

std::vector<double>
MNist::Loader::loadLabelsFile(const std::string &labelsPath) {
  std::ifstream labelsFile{labelsPath, std::ios::binary | std::ios::in};

  // testing number which should always be 2049
  unsigned int magicNumber{readU32(labelsFile, labelsPath)};
  if (magicNumber != 2049)
    throw MNist::Exception{
        "MNist::Loader::loadLabelsFile(const std::string&)",
        "Invalid file format for image label file: " + labelsPath +
            "\n Magic number mismatch (expected 2049)"};

  // Number of image labels (each one is a byte, so also length of remaining
  // file)
  unsigned int size{readU32(labelsFile, labelsPath)};

  // Actual data
  std::vector<double> labels{readBytes(labelsFile, size, labelsPath)};

  return labels;
}

std::vector<Math::Matrix<double>>
MNist::Loader::loadImagesFile(const std::string &imagesPath) {
  std::ifstream imagesFile{imagesPath, std::ios::binary | std::ios::in};

  // testing number which should always be 2051
  unsigned int magicNumber{readU32(imagesFile, imagesPath)};
  if (magicNumber != 2051)
    throw MNist::Exception{
        "MNist::Loader::loadImagesFile(const std::string&)",
        "Invalid file format for images file: " + imagesPath +
            "\n Magic number mismatch (expected 2051)"};

  // Number of images
  unsigned int size{readU32(imagesFile, imagesPath)};

  // Number of rows in each image
  unsigned int rows{readU32(imagesFile, imagesPath)};

  // Number of columns in each image
  unsigned int cols{readU32(imagesFile, imagesPath)};

  // Plain image data
  std::vector<double> imageData{
      readBytes(imagesFile, size * rows * cols, imagesPath)};

  std::vector<Math::Matrix<double>> images{};
  images.reserve(size);

  for (unsigned int i{}; i < size; ++i) {
    images.push_back(
        Math::Matrix<double>(rows, cols, &imageData[i * rows * cols]));
  }

  return images;
}

std::vector<double> MNist::Loader::readBytes(std::ifstream &file,
                                             const unsigned int length,
                                             const std::string &filename) {
  std::vector<char> bytes(length);
  if (!file.read(&bytes[0], length))
    throw MNist::Exception{"MNist::Loader::readBytes(std::ifstream&, const "
                           "unsigned int, const std::string&)",
                           "Can't read file " + filename +
                               ": file size smaller then needed to read " +
                               std::to_string(length) + " bytes."};

  std::vector<double> doubleBytes(length);

  std::transform(bytes.begin(), bytes.end(), doubleBytes.begin(),
                 [](char c) { return static_cast<double>(c); });

  return doubleBytes;
}

unsigned int MNist::Loader::readU32(std::ifstream &file,
                                    const std::string &filename) {
  char bytes[4]{};
  if (!file.read(bytes, 4))
    throw MNist::Exception{
        "MNist::Loader::readU32(std::ifstream&, const std::string&)",
        "Can't read file " + filename + ": reached end"};
  unsigned int value{
      static_cast<unsigned int>(static_cast<unsigned char>(bytes[3]) |
                                (static_cast<unsigned char>(bytes[2]) << 8) |
                                (static_cast<unsigned char>(bytes[1]) << 16) |
                                (static_cast<unsigned char>(bytes[0]) << 24))};
  return value;
}
