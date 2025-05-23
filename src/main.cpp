#include "math/matrixBase.h"
#include "math/random.h"
#include "mnist/loader.h"

#include "ann/layers/categoricalLossSoftmax.h"
#include "ann/layers/dense.h"
#include "ann/layers/dropout.h"

#include "ann/activations/leakyRelu.h"

#include "ann/optimizers/adam.h"

#include <iostream>
#include <stdexcept>

#include <chrono>

class Timer {
private:
  // Type aliases to make accessing nested type easier
  using Clock = std::chrono::high_resolution_clock;
  using Second = std::chrono::duration<double, std::ratio<1>>;

  std::chrono::time_point<Clock> m_beg{Clock::now()};

public:
  void reset() { m_beg = Clock::now(); }

  double elapsed() const {
    return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
  }
};

template <typename T> void printMatrixImage(const Math::MatrixBase<T> &m) {
  for (size_t row{}; row < m.rows(); ++row) {
    for (size_t col{}; col < m.cols(); ++col)
      std::cout << "\033[38;2;" << static_cast<unsigned int>(m[row, col] * 255)
                << ';' << static_cast<unsigned int>(m[row, col] * 255) << ';'
                << static_cast<unsigned int>(m[row, col] * 255) << "m█\033[0m";
    std::cout << '\n';
  }
}

std::string formatTime(double seconds) {
  if (seconds >= 1.0)
    return std::to_string(static_cast<unsigned long long>(seconds)) + "s";

  const unsigned long long ns = static_cast<unsigned long long>(seconds * 1e9);
  if (ns < 1000)
    return std::to_string(ns) + "ns";

  const unsigned long long us = ns / 1000;
  if (us < 1000)
    return std::to_string(us) + "us";

  const unsigned long long ms = us / 1000;
  if (ms < 1000)
    return std::to_string(ms) + "ms";

  // Fallback: round down to 0s
  return "0s";
}

int main() {
  try {
    auto loader{std::make_unique<MNist::Loader>(
        "data/train-labels-idx1-ubyte", "data/train-images-idx3-ubyte",
        "data/t10k-labels-idx1-ubyte", "data/t10k-images-idx3-ubyte")};
    std::array<MNist::Loader::DataPair, 2> data{loader->loadData()};

    auto trainingImages{std::get<1>(data[0])};
    auto trainingLabels{std::get<0>(data[0])};
    auto testingImages{std::get<1>(data[1])};
    auto testingLabels{std::get<0>(data[1])};

    constexpr unsigned int imageRows{28};
    constexpr unsigned int imageCols{28};
    constexpr unsigned short layer1Neurons{32};
    constexpr unsigned short layer2Neurons{16};
    constexpr unsigned short outputNeurons{10};
    constexpr unsigned int batchSize{64};
    constexpr unsigned int trainingSize{60'000};
    constexpr unsigned int epochs{1};
    constexpr float learningRate{2e-2};
    constexpr float learningRateDecay{3e-4};
    constexpr float learningRateMomentum{0.3};

    auto dense1{std::make_unique<Layer::Dense>(
        imageRows * imageCols, layer1Neurons, Layer::WeightInit::He, 0, 0,
        1e-5f, 1e-5f)};

    auto activation1{std::make_unique<Activation::LeakyReLU>(1e-2)};

    auto dropout1{std::make_unique<Layer::Dropout>(1e-1)};

    auto dense2{std::make_unique<Layer::Dense>(layer1Neurons, layer2Neurons,
                                               Layer::WeightInit::He, 0, 0,
                                               1e-5f, 1e-5f)};

    auto activation2{std::make_unique<Activation::LeakyReLU>(1e-2)};

    auto dropout2{std::make_unique<Layer::Dropout>(5e-2)};

    auto outputLayer{std::make_unique<Layer::Dense>(
        layer2Neurons, outputNeurons, Layer::WeightInit::He)};

    auto outputSoftmaxLoss{std::make_unique<Layer::CategoricalLossSoftmax>()};

    auto optimizer{
        std::make_unique<Optimizers::Adam>(learningRate, learningRateDecay)};

    Timer trainingTimer{};
    Timer displayTimer{};

    constexpr size_t batches{trainingSize / batchSize};

    // Set up floating point printing for training updates
    std::cout << std::fixed << std::setprecision(4);

    for (size_t epoch{}; epoch < epochs; ++epoch) {

      // Set up batch sequence
      std::vector<size_t> batchSequence(trainingSize / batchSize);
      std::iota(batchSequence.begin(), batchSequence.end(), 0);
      std::shuffle(batchSequence.begin(), batchSequence.end(),
                   Math::Random::mt);

      trainingTimer.reset();
      for (size_t batch{}; batch < trainingSize / batchSize; ++batch) {

        // FORWARD PASS

        // Get batchSize training images and labels
        auto inputData{
            trainingImages->view(batchSequence[batch] * batchSize,
                                 (batchSequence[batch] + 1) * batchSize)};
        auto inputCorrect{
            trainingLabels->view(batchSequence[batch] * batchSize,
                                 (batchSequence[batch] + 1) * batchSize)};

        dense1->forward(inputData);
        activation1->forward(dense1->output());
        dropout1->forward(activation1->output());
        dense2->forward(dropout1->output());
        activation2->forward(dense2->output());
        dropout2->forward(activation2->output());
        outputLayer->forward(dropout2->output());
        outputSoftmaxLoss->forward(outputLayer->output(), inputCorrect);

        float dataLoss{outputSoftmaxLoss->mean()};
        float regularizationLoss{
            outputSoftmaxLoss->regularizationLoss(*dense1) +
            outputSoftmaxLoss->regularizationLoss(*dense2)};
        float loss{dataLoss + regularizationLoss};

        // BACKWARD PASS
        outputSoftmaxLoss->backward();
        outputLayer->backward(outputSoftmaxLoss->dinputs());
        dropout2->backward(outputLayer->dinputs());
        activation2->backward(dropout2->dinputs());
        dense2->backward(activation2->dinputs());
        dropout1->backward(dense2->dinputs());
        activation1->backward(dropout1->dinputs());
        dense1->backward(activation1->dinputs());

        optimizer->preUpdate();
        optimizer->updateParams(*outputLayer);
        optimizer->updateParams(*dense2);
        optimizer->updateParams(*dense1);
        optimizer->postUpdate();

        // Display information every about half second, or at the first/final
        // batch
        if (displayTimer.elapsed() >= 0.5 ||
            batch + 1 == trainingSize / batchSize || batch == 0) {
          std::cout << '\r' << batch + 1 << '/' << trainingSize / batchSize
                    << '\t' << static_cast<size_t>(trainingTimer.elapsed())
                    << "s " << formatTime(trainingTimer.elapsed() / (batch + 1))
                    << "/step \taccuracy: " << outputSoftmaxLoss->accuracy()
                    << " - loss: " << loss << " (data loss: " << dataLoss
                    << ", reg loss: " << regularizationLoss
                    << ") - lr: " << optimizer->learningRate()
                    << "                 " << std::flush;

          displayTimer.reset();
        }
      }

      std::cout << '\n';
    }

    dense1->forward(testingImages);
    activation1->forward(dense1->output());
    dense2->forward(activation1->output());
    activation2->forward(dense2->output());
    outputLayer->forward(activation2->output());
    outputSoftmaxLoss->forward(outputLayer->output(), testingLabels);

    std::cout << "Test loss: " << outputSoftmaxLoss->mean()
              << "\nTest accuracy: " << outputSoftmaxLoss->accuracy() << '\n';
  } catch (std::runtime_error &e) {
    std::cout << "An error occured: " << e.what() << '\n';
  } catch (...) {
    std::cout << "An unknown error occured\n";
  }

  return 0;
}
