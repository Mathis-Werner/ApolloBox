#include "AudioTools.h"
#include "AudioTools/AudioLibs/VS1053Stream.h"

URLStream url("Martin Router King","19711820478148934850");  // or replace with ICYStream to get metadata
VS1053Stream vs1053; // final output
StreamCopy copier(vs1053, url); // copy url to decoder

// NTP Server details for time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;       // Adjust for your timezone (e.g., 3600 for GMT+1)
const int   daylightOffset_sec = 0;  // Adjust if you observe daylight saving

struct tm previousTime;  // Store the previous time snapshot

// Setup switch
const int SWITCH_PIN = 21;

void setup(){
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);  

  // setup vs1053
  auto cfg = vs1053.defaultConfig();
  cfg.is_encoded_data = true; // vs1053 is accepting encoded data
  // Use your custom pins or define in AudioCodnfig.h
  cfg.cs_pin = 5; 
  cfg.dcs_pin = 16;
  cfg.dreq_pin = 4;
  //cfg.reset_pin = VS1053_RESET;
  vs1053.begin(cfg);

  // mp3 radio
  url.begin("https://edge63.radio.apolloradio.de/apollo-radio/stream/mp3?aggregator=smk-m3u-mp3","audio/mp3");
  //http://streaming.fueralle.org:8000/coloradio_160.mp3

  // Initialize NTP for time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Wait for time to be set
  struct tm timeInfo;
  if(!getLocalTime(&timeInfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  // Set initial stream based on time
  // When time between 00:00 and 12:00 enable apollo
  if (timeInfo.tm_hour >= 0 && timeInfo.tm_hour <= 11) {
    url.begin("https://edge63.radio.apolloradio.de/apollo-radio/stream/mp3?aggregator=smk-m3u-mp3","audio/mp3");
  } else {
    url.begin("http://streaming.fueralle.org:8000/coloradio_160.mp3","audio/mp3");
  }
  previousTime = timeInfo;

  // Enable switch pin
  pinMode(SWITCH_PIN, INPUT_PULLUP);
}

void loop(){
  // Section to change stream url for apollo or colo
  struct tm currentTime;
  if(getLocalTime(&currentTime)){
    // Monday tuesday apollo shedule
    if ((currentTime.tm_wday == 1 || currentTime.tm_wday == 2) && previousTime.tm_hour == 23 && previousTime.tm_min == 59 &&
        currentTime.tm_hour == 0 && currentTime.tm_min == 0) {
      url.begin("https://edge63.radio.apolloradio.de/apollo-radio/stream/mp3?aggregator=smk-m3u-mp3","audio/mp3");
    }
    // Wednesday saturday apollo shedule
    if ((currentTime.tm_wday >= 3 || currentTime.tm_wday <= 6) && currentTime.tm_wday != 0 &&
        previousTime.tm_hour == 0 && previousTime.tm_min == 59 &&
        currentTime.tm_hour == 1 && currentTime.tm_min == 0) {
      url.begin("https://edge63.radio.apolloradio.de/apollo-radio/stream/mp3?aggregator=smk-m3u-mp3","audio/mp3");
    }
    // Sunday apollo shedule
    if ((currentTime.tm_wday == 0) && previousTime.tm_hour == 1 && 
        previousTime.tm_min == 59 &&
        currentTime.tm_hour == 2 && currentTime.tm_min == 0) {
      url.begin("https://edge63.radio.apolloradio.de/apollo-radio/stream/mp3?aggregator=smk-m3u-mp3","audio/mp3");
    }
    // Monday to friday colo shedule
    if ((currentTime.tm_wday >= 1 || currentTime.tm_wday <= 5) && currentTime.tm_wday != 0 &&
               previousTime.tm_hour == 11 && previousTime.tm_min == 59 &&
               currentTime.tm_hour == 12 && currentTime.tm_min == 0) {
      url.begin("http://streaming.fueralle.org:8000/coloradio_160.mp3","audio/mp3");
    }
    // Saturday and suinday colo shedule
    if ((currentTime.tm_wday == 0 && currentTime.tm_wday == 6) &&
               previousTime.tm_hour == 7 && previousTime.tm_min == 59 &&
               currentTime.tm_hour == 8 && currentTime.tm_min == 0) {
      url.begin("http://streaming.fueralle.org:8000/coloradio_160.mp3","audio/mp3");
    }
    // Update previous time with current time
    previousTime = currentTime;
  }

  // Read the state of the switch
  int switchState = digitalRead(SWITCH_PIN);

  if (switchState == HIGH) {
    copier.copy();
  }
}