/*
 ==============================================================================

 PlayableBuffer.h
 Created: 6 Jun 2016 7:45:50pm
 Author:  Martin Hermant

 ==============================================================================
 */

#ifndef PLAYABLEBUFFER_H_INCLUDED
#define PLAYABLEBUFFER_H_INCLUDED

#include "JuceHeader.h"
#include "DebugHelpers.h"




class PlayableBuffer {

    public :
    PlayableBuffer(int numChannels,int numSamples):
    loopSample(numChannels,numSamples),
    recordNeedle(0),
    playNeedle(0),isJumping(false),
    state(BUFFER_STOPPED),
    lastState(BUFFER_STOPPED)
    {

//        for (int j = 0 ; j < numSamples ; j++){int p = 44;float t = (j%p)*1.0/p;float v = t;
//            for(int i = 0 ; i < numChannels ; i++){loopSample.addSample(i, j, v);}
//        }
                loopSample.clear();
    }

    bool writeAudioBlock(const AudioBuffer<float> & buffer){

        if (recordNeedle + buffer.getNumSamples()> loopSample.getNumSamples()) {
            jassertfalse;
            return false;
        }
        else{
            for (int i = loopSample.getNumChannels() - 1; i >= 0; --i) {
                loopSample.copyFrom(i, recordNeedle, buffer, i, 0, buffer.getNumSamples());
            }
            recordNeedle += buffer.getNumSamples();
        }

        return true;
    }

    int getNumSamples() const{return loopSample.getNumSamples();}

    void readNextBlock(AudioBuffer<float> & buffer){

        if ((playNeedle + buffer.getNumSamples()) > recordNeedle+1)
        {

            //assert false for now see above
            jassertfalse;
            //            // crop buffer to ensure not coming back
            //            recordNeedle = (playNeedle + buffer.getNumSamples());
            int firstSegmentLength = recordNeedle - playNeedle;
            int secondSegmentLength = buffer.getNumSamples() - firstSegmentLength;
            for (int i = buffer.getNumChannels() - 1; i >= 0; --i) {
                int maxChannelFromRecorded = jmin(loopSample.getNumChannels() - 1, i);
                buffer.copyFrom(i, 0, loopSample, maxChannelFromRecorded, playNeedle, firstSegmentLength);
                buffer.copyFrom(i, 0, loopSample, maxChannelFromRecorded, 0, secondSegmentLength);
            }
            playNeedle = secondSegmentLength;


        }

        // stitch audio jumps by quick fadeIn/Out
        if(isJumping && playNeedle!=startJumpNeedle){
            LOG("a:jump "<<startJumpNeedle <<","<< playNeedle);
            const int halfBlock =  buffer.getNumSamples()/2;
            for (int i = buffer.getNumChannels() - 1; i >= 0; --i) {
                int maxChannelFromRecorded = jmin(loopSample.getNumChannels() - 1, i);
                buffer.copyFrom(i, 0, loopSample, maxChannelFromRecorded, startJumpNeedle, halfBlock);
                buffer.applyGainRamp(i, 0, halfBlock, 1.0f, 0.0f);
                buffer.copyFrom(i, halfBlock, loopSample, maxChannelFromRecorded, playNeedle+halfBlock, halfBlock);
                buffer.applyGainRamp(i, 0, halfBlock, 0.0f, 1.0f);

            }


        }
        else{
            for (int i = buffer.getNumChannels() - 1; i >= 0; --i) {
                int maxChannelFromRecorded = jmin(loopSample.getNumChannels() - 1, i);
                buffer.copyFrom(i, 0, loopSample, maxChannelFromRecorded, playNeedle, buffer.getNumSamples());
            }
        }
        isJumping = false;

        playNeedle += buffer.getNumSamples();
        playNeedle %= recordNeedle+1;





    }


    void setPlayNeedle(int n){
        if(playNeedle!=n){
            isJumping = true;
            startJumpNeedle = playNeedle;
        }

        playNeedle = n;


    }

    void cropEndOfRecording(int sampletoRemove){
        jassert(sampletoRemove<recordNeedle);
        recordNeedle-=sampletoRemove;
    }

    void fadeInOut(int fadeNumSamples,double mingain){
        if (fadeNumSamples>0 && recordNeedle>2 * fadeNumSamples) {
            for (int i = loopSample.getNumChannels() - 1; i >= 0; --i) {
                loopSample.applyGainRamp(i, 0, fadeNumSamples, mingain, 1);
                loopSample.applyGainRamp(i, recordNeedle - fadeNumSamples, fadeNumSamples, 1, mingain);
            }
        }
    }
    bool isFirstPlayingFrameAfterRecord(){
        return lastState == BUFFER_RECORDING && state == BUFFER_PLAYING;
    }
    bool isStopping() const{
        return (lastState == BUFFER_PLAYING ) && (state==BUFFER_STOPPED);
    }
    bool isRecording() const{
        return state == BUFFER_RECORDING;
    }
    bool firstRecordedFrame() const{
        return state == BUFFER_RECORDING && (lastState!=BUFFER_RECORDING);
    }
    void startRecord(){
        recordNeedle = 0;
    }

    void startPlay(){
        playNeedle = 0;
    }

    bool checkTimeAlignment(uint64 curTime){

        if(state == BUFFER_PLAYING && playNeedle>=0 && recordNeedle>0){

            const int minQuantifiedFraction = recordNeedle+1;

            int globalPos =(curTime%minQuantifiedFraction);
            int localPos =(playNeedle%minQuantifiedFraction);
            if(globalPos!=localPos){
                isJumping = true;
                startJumpNeedle = playNeedle;
                playNeedle = (playNeedle - localPos) + globalPos;

            }
        }

        return !isJumping;
    }


    enum BufferState {
        BUFFER_STOPPED = 0,
        BUFFER_PLAYING,
        BUFFER_RECORDING

    };

    void setState(BufferState newState){
        //        lastState = state;
        stateChanged |=newState!=state;
        switch (newState){
            case BUFFER_RECORDING:
                recordNeedle = 0;
                playNeedle = -1;
                break;
            case BUFFER_PLAYING:
                playNeedle = 0;
                break;
            case BUFFER_STOPPED:
                break;
        }
        state = newState;

    }

    void endProcessBlock(){
        lastState = state;
        stateChanged =false;
    }

    BufferState getState() const{
        return state;
    }

    BufferState getLastState() const{
        return lastState;
    }

    bool isOrWasPlaying() const{
        return (state==BUFFER_PLAYING || lastState==BUFFER_PLAYING) &&  recordNeedle>0 && loopSample.getNumSamples();
    }
    int getRecordedLength() const{return recordNeedle+1;}

    int getPlayPos() const{return playNeedle;}


    bool stateChanged;

    int getStartJumpPos() const{return startJumpNeedle;}
    
    
    
private:
    
    BufferState state;
    BufferState lastState;
    bool isJumping;
    AudioSampleBuffer loopSample;
    
    int recordNeedle,playNeedle,startJumpNeedle;
    
};



#endif  // PLAYABLEBUFFER_H_INCLUDED
