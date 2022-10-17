class SMA {
    private:
        int size;
        float * samples;
        int index;
        float med;

    public:
        SMA(int _size):
        size(_size),
        samples(new float[size]),
        index(0),
        med(0) {
            for (int i = 0; i < size; i++)
                samples[i] = 0;
        }

        ~SMA() {
            delete[] samples;
        }

    float updateSamples(float nSample) {
        med += (nSample - samples[index]) / size;
        samples[index] = nSample;
        index = (index + 1) % size;

        return med;
    }
};
