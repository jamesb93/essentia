#include <iostream>
#include <essentia/algorithmfactory.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/scheduler/network.h>
#include "credit_libav.h"

using namespace std;
using namespace essentia;
using namespace essentia::streaming;
using namespace essentia::scheduler;

int main(int argc, char* argv[]) {

    string audioFilename = argv[1];
    string outputFilename = argv[2];


    // register the algorithms in the factory(ies)
    essentia::init();

    /////// PARAMS //////////////
    int framesize = 8192;
    int hopsize = 64;
    int sr = 44100;
    vector<string> statistics = {
        "min",
        "max",
        "median",
        "mean",
        "stdev",
    };
    
    // instanciate facgory and create algorithms:
    streaming::AlgorithmFactory& factory = streaming::AlgorithmFactory::instance();

    Algorithm* audioload        = factory.create("MonoLoader",
                                           "sampleRate", sr,
                                           "filename", audioFilename);
    Algorithm* frameCutter      = factory.create("FrameCutter",
                                           "frameSize", framesize,
                                           "hopSize", hopsize,
                                           "startFromZero", true );
    Algorithm* window           = factory.create("Windowing", "type", "hann");
    Algorithm* spectrum         = factory.create("Spectrum");
    Algorithm* yin              = factory.create("PitchYinFFT", "frameSize", framesize, "sampleRate", sr);
    Algorithm* specPeaks        = factory.create("SpectralPeaks", "minFrequency", 1);
    Algorithm* harmPeaks        = factory.create("HarmonicPeaks");
    Algorithm* dissonance       = factory.create("Dissonance");
    Algorithm* tristim          = factory.create("Tristimulus");
    Algorithm* inharm           = factory.create("Inharmonicity");
    Algorithm* specContrast     = factory.create("SpectralContrast", "frameSize", framesize);
    Algorithm* specComplexity   = factory.create("SpectralComplexity");
    Algorithm* pitchSalience    = factory.create("PitchSalience");
    Algorithm* strongPeak       = factory.create("StrongPeak");
    Algorithm* flux             = factory.create("Flux");
    Algorithm* flatness         = factory.create("FlatnessDB");
    Algorithm* moments          = factory.create("CentralMoments");
    Algorithm* zc               = factory.create("ZeroCrossingRate");
    Algorithm* entropy          = factory.create("Entropy");
    Algorithm* dynComplexity    = factory.create("DynamicComplexity");
    
    // data storage
    Pool pool;

    /////////// CONNECTING THE ALGORITHMS ////////////////

    // audio -> framecutter
    audioload->output("audio")              >>  frameCutter->input("signal");

    // Spectrum
    frameCutter->output("frame")            >>  window->input("frame");
    window->output("frame")                 >>  spectrum->input("frame");

    // YIN
    spectrum->output("spectrum")            >>  yin->input("spectrum");
    yin->output("pitch")                    >>  PC(pool, "fundamental");
    yin->output("pitchConfidence")          >>  PC(pool, "pitch_confidence");

    // Spectral Peaks
    spectrum->output("spectrum")            >>  specPeaks->input("spectrum");

    // Spectral Complexity
    spectrum->output("spectrum")                    >>  specComplexity->input("spectrum");
    specComplexity->output("spectralComplexity")    >> PC(pool, "spectral_complexity");

    // Spectral Contrast
    spectrum->output("spectrum")                >>  specContrast->input("spectrum");
    specContrast->output("spectralContrast")    >>  PC(pool, "spectral_contrast");
    specContrast->output("spectralValley")      >>  PC(pool, "spectral_valley");

    // Spectral Entropy
    spectrum->output("spectrum")            >>  entropy->input("array");
    entropy->output("entropy")              >>  PC(pool, "spectral_entropy");


    // Spectral Flatness
    spectrum->output("spectrum")            >>  flatness->input("array");
    flatness->output("flatnessDB")          >>  PC(pool, "spectral_flatness_measure");

    // Spectral Moments
    spectrum->output("spectrum")            >>  moments->input("array");
    moments->output("centralMoments")       >>  PC(pool, "spectral_moments");

    // Strong Peak
    spectrum->output("spectrum")            >>  strongPeak->input("spectrum");
    strongPeak->output("strongPeak")        >>  PC(pool, "strong_peak");

    // Flux
    spectrum->output("spectrum")            >>  flux->input("spectrum");
    flux->output("flux")                    >>  PC(pool, "spectral_flux");

    // Pitch Salience
    spectrum->output("spectrum")            >>  pitchSalience->input("spectrum");
    pitchSalience->output("pitchSalience")  >>  PC(pool, "pitch_salience");
    
    // Dissonance
    specPeaks->output("frequencies")        >>  dissonance->input("frequencies");
    specPeaks->output("magnitudes")         >>  dissonance->input("magnitudes");
    dissonance->output("dissonance")        >>  PC(pool, "dissonance");
    
    // Harmonic Peaks
    specPeaks->output("frequencies")        >>  harmPeaks->input("frequencies");
    specPeaks->output("magnitudes")         >>  harmPeaks->input("magnitudes");
    yin->output("pitch")                    >>  harmPeaks->input("pitch");

    // Tristimulus
    harmPeaks->output("harmonicFrequencies")          >>  tristim->input("frequencies");
    harmPeaks->output("harmonicMagnitudes")           >>  tristim->input("magnitudes");
    tristim->output("tristimulus")                    >>  PC(pool, "tristimulus");

    // Inharmonicity
    harmPeaks->output("harmonicFrequencies")          >>  inharm->input("frequencies");
    harmPeaks->output("harmonicMagnitudes")           >>  inharm->input("magnitudes");
    inharm->output("inharmonicity")                   >>  PC(pool, "inharmonicity");

    // Zero Crossing
    frameCutter->output("frame")            >>  zc->input("signal");
    zc->output("zeroCrossingRate")          >>  PC(pool, "zerox");

    // Dynamic Complexity
    audioload->output("audio")                  >>  dynComplexity->input("signal");
    dynComplexity->output("dynamicComplexity")  >>  PC(pool, "dynamic_complexity");
    dynComplexity->output("loudness")           >>  PC(pool, "loudness");
    
    Network n(audioload);
    pool.clear();
    n.reset();
    n.run();

    standard::Algorithm* aggregator = standard::AlgorithmFactory::create("PoolAggregator",
                                                                            "defaultStats", statistics);

    standard::Algorithm* output     = standard::AlgorithmFactory::create("YamlOutput",
                                                                            "format", "json",
                                                                            "writeVersion", false, 
                                                                            "filename", outputFilename);
    Pool poolStats;

    aggregator->input("input").set(pool);
    aggregator->output("output").set(poolStats);
    output->input("pool").set(poolStats);

    aggregator->compute();
    output->compute();
    n.clear();
    delete output;
    delete aggregator;
    essentia::shutdown();
    return 0;
}
