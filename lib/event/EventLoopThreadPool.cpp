#include "../../include/event/EventLoopThreadPool.hpp"
#include "../../include/event/Channel.hpp"
#include "../../include/event/EventLoop.hpp"

namespace Collie {
namespace Event {

EventLoopThreadPool::EventLoopThreadPool(const size_t threadNum)
    : threadNum(threadNum) {}

void
EventLoopThreadPool::startLoop(
    std::vector<std::shared_ptr<Channel>> baseChannel) {
    if(!runInThread) {
        // default
        runInThread = [this, baseChannel] {
            std::shared_ptr<EventLoopThread> eventLoopThread(
                new EventLoopThread);
            {
                std::lock_guard<std::mutex> lock(eventLoopThreadMtx);
                eventLoopThreads.push_back(eventLoopThread);
            }

            // one loop per thread
            std::shared_ptr<EventLoop> eventLoop(new EventLoop);

            // insert base channels to loop
            for(auto channel : baseChannel) {
                auto newChannel = channel->getCopyWithoutEventLoop();
                newChannel->setEventLoop(eventLoop);
                newChannel->update();
            }

            // loop
            while(true) {
                std::vector<std::shared_ptr<Channel>> channels;
                // get new channel or terminate
                // Non blocking
                if(eventLoopThread->mtx.try_lock() &&
                   (terminate || !eventLoopThread->channels.empty())) {
                    if(terminate && channels.empty()) return; // exit
                    channels.swap(eventLoopThread->channels);
                    eventLoopThread->mtx.unlock();
                }
                // insert channel
                for(auto channel : channels) {
                    channel->setEventLoop(eventLoop);
                    channel->update();
                }
                // block
                eventLoop->loopOne();
            }
        };
    }
    runInThread();
}

std::shared_ptr<EventLoopThreadPool::EventLoopThread>
EventLoopThreadPool::getNextLoop() {
    std::lock_guard<std::mutex> lock(eventLoopThreadMtx);
    static auto it = eventLoopThreads.begin();
    if(it == eventLoopThreads.end()) {
        it = eventLoopThreads.begin();
    }
    return *(it++);
}

void
EventLoopThreadPool::pushChannel(std::shared_ptr<Channel> channel) {
    // insert to NEXT loop
    auto loop = getNextLoop();
    {
        std::lock_guard<std::mutex> lock(loop->mtx);
        loop->channels.push_back(channel);
    }
}
}
}