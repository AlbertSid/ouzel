// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_AUDIO_NODE_HPP
#define OUZEL_AUDIO_NODE_HPP

#include <cstdint>
#include <algorithm>
#include <vector>

namespace ouzel
{
    namespace audio
    {
        class Audio;

        class Node
        {
        public:
            explicit Node(Audio& initAudio):
                audio(initAudio)
            {
            }

            virtual ~Node()
            {
                if (parent)
                    parent->removeChild(*this);
            }

            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;

            Node(Node&&) = delete;
            Node& operator=(Node&&) = delete;

            void addChild(Node& child);
            void removeChild(Node& child);

        private:
            Audio& audio;
            uintptr_t objectId = 0;
            Node* parent = nullptr;
            std::vector<Node*> children;
        };
    } // namespace audio
} // namespace ouzel

#endif // OUZEL_AUDIO_NODE_HPP
