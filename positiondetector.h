#ifndef POSITIONDETECTOR_H
#define POSITIONDETECTOR_H

#include <iostream>
#include <array>
#include <iomanip>
#include <functional>
#include <cmath>   // Needed for fabs


namespace PositionDetectorName {   // ===== RENAMED NAMESPACE =====

    class PositionDetector{        // ===== RENAMED CLASS =====
        
        public:

        /**
         * @brief Checks if bat is held upright (starting position).
         *
         * ===== DIFFERENCE FROM ORIGINAL IMUMATHS =====
         * - No audio logic
         * - No ALSA dependency
         * - No drum axis thresholds
         * - Only detects upright position
         */
        void CheckPosition(float X, float Y, float Z);  // ===== RENAMED FUNCTION =====

        // Represents "upright confirmed" state
        bool UprightConfirmed = false;   // ===== RENAMED FROM Pause =====

        int Counter = 0;  // Counts stable samples

        /**
         * ===== MODIFIED CALLBACK =====
         * Previously: AudioTrigger(FilePath)
         * Now: generic event trigger (no audio dependency)
         */
        struct Callback{
            virtual void OnUprightDetected() = 0;  // ===== RENAMED =====
            virtual ~Callback(){};
        };

        void RegisterCallback(Callback* cb){
            callback = cb;
        }

        /**
         * Accessor for upright state
         */
        bool IsUpright() const{
            return UprightConfirmed;
        }

        #ifdef UNIT_TEST 
        
        bool HasCallback() const {
            return callback != nullptr;
        }

        Callback* GetCallback() const {
            return callback;
        }

        #endif

        private:
        
        Callback* callback = nullptr;

        // ===== REMOVED FROM ORIGINAL =====
        // - ALSA include
        // - AudioCallback struct
        // - PlayFileCallback
        // - LastFilePlayed
        // ================================
    };

}

#endif
