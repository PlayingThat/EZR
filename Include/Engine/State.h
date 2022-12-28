//
// Created by maxbr on 26.05.2020.
//

#pragma  once

#include "Defs.h"
#include "Camera.h"

#include <vector>
#include <algorithm>


// Subscribers can use this class to get notified when the window is resized
// Classes can inherit from this class and implement the windowSizeChangeCallback function to get notified when the window is resized
class SizeCallbackSubscriber
{
public:
    virtual void onSizeChanged(int width, int height) {};
};

class Subject
{
public:
    
};

class State
{
public:
    State();

    State(size_t width, size_t height);
    ~State();

    std::shared_ptr<Camera> getCamera() const;

    void setCamera(std::shared_ptr<Camera> mCamera);

    size_t getWidth() const;

    size_t getHeight() const;

    double getDeltaTime() const;

    void setDeltaTime(double mDeltaTime);

    double getTime() const;

    void setTime(double mTime);

    ////////////////////////////////////////////
    // Size callback functions

    // Attach a subscriber to the list of window size callback subscribers
    void attachWindowSizeChangeCallback(SizeCallbackSubscriber* observer);
    // Detach a subscriber from the list of window size callback subscribers (probably not needed)
    void detachWindowSizeChangeCallback(SizeCallbackSubscriber* observer);
    // Notify all subscribers that the window size has changed
    void notifyWindowSizeChange(int width, int height);

private:
    std::shared_ptr<Camera> m_camera;
    size_t m_width, m_height;
    double m_deltaTime;
    double m_time;

    // List of subscribers for resize window callback
    std::vector<SizeCallbackSubscriber*> m_sizeCallbackSubscribers;

};