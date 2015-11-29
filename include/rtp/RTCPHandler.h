/* 
 * File:   RTCPHandler.h
 * Author: daniel
 *
 * Created on November 5, 2015, 11:04 AM
 */

#ifndef RTCPHANDLER_H
#define	RTCPHANDLER_H

#include <memory>
#include <thread>
#include <chrono> // clock, tick
#include "NetworkWrapper.h"
#include "ParticipantDatabase.h"
#include "RTCPPackageHandler.h"
#include "ConfigurationMode.h"
#include "Statistics.h"

/*!
 * The RTCPHandler manages a thread for RTCP-communication.
 * 
 * The port occupied by RTCP is per standard the RTP-port +1
 */
class RTCPHandler
{
public:
    RTCPHandler(std::unique_ptr<NetworkWrapper>&& networkWrapper, const std::shared_ptr<ConfigurationMode> configMode, 
                const std::function<void ()> startCallback, const std::function<void ()> stopCallback);
    ~RTCPHandler();
    
    /*!
     * Shuts down the receive-thread
     */
    void shutdown();

    /*!
     * Starts the receive-thread
     */
    void startUp();
    
private:
    const std::unique_ptr<NetworkWrapper> wrapper;
    const std::shared_ptr<ConfigurationMode> configMode;
    const std::function<void ()> startAudioCallback;
    const std::function<void ()> stopCallback;
    RTCPPackageHandler rtcpHandler;
    
    std::thread listenerThread;
    bool threadRunning = false;
    
    //send SR every X seconds
    static const std::chrono::seconds sendSRInterval;
    std::chrono::system_clock::time_point lastSRReceived;
    std::chrono::system_clock::time_point lastSRSent;
    
    /*!
     * Method called in the parallel thread, receiving RTCP-packages and handling them
     */
    void runThread();
    
    /*!
     * This method handles received RTCP packages and is called only from #runThread()
     */
    void handleRTCPPackage(void* receiveBuffer, unsigned int receivedSize);
    
    void shutdownInternal();
    
    /*!
     * Sends a Source Description-package, only called from #runThread()
     */
    void sendSourceDescription();
    
    /*!
     * Sends a Source BYE-package, only called from #runThread()
     */
    void sendByePackage();

    /*!
     * Creates a SR package to be used in a compound package
     */
    const void* createSenderReport(unsigned int offset = 0);
    
    /*!
     * Creates a SDES package to be used in a compound package
     */
    const void* createSourceDescription(unsigned int offset = 0);
    
    /*!
     * \return the fraction of lost-packages as 1/256
     */
    uint8_t calculateFractionLost();
};

#endif	/* RTCPHANDLER_H */
