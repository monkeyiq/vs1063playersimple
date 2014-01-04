/**
 * \file vs1063x,
 *
 * This started life as the FilePlayer.ino SFEMP3Shield example.
 *
 *
 * Support has been added for buttons, and a screen. 
 * 
 */

#include <Streaming.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SPI.h>
#include <Wire.h>
#include <SwitchMCP.h>
#include <ADXL362.h>

#include <SimpleTimer.h>
SimpleTimer timer;

void parse_menu(byte key_command);
void SerialPrintPaddedNumber(int16_t value, int8_t digits );



/* byte attinyScreenDriverChipSelect = 3; */
const byte I2CADDR_MCP27017 = 0x20;
const byte buttonWire_PlayPause = 0;
const byte buttonWire_nextTrack = 1;
const byte buttonWire_tmp       = 2;

const byte outputWire_attinyScreenDriverChipSelect = 0;
      byte outputWire_ramChipSelect                = 2;
const byte outputWire_vs1063controlChipSelect      = 3;
const byte outputWire_vs1063dataChipSelect         = 4;

MCPReader reader( I2CADDR_MCP27017 );
MCPWriter writer( I2CADDR_MCP27017 );

SwitchMCP bPlayPause( reader, buttonWire_PlayPause );
SwitchMCP bNextTrack( reader, buttonWire_nextTrack );
SwitchMCP bTemp     ( reader, buttonWire_tmp       );

#include <Shim_CharacterOLEDSPI3.h>
Shim_CharacterOLEDSPI3 lcd(new ChipSelect( &writer, outputWire_attinyScreenDriverChipSelect ),
                           6, 7, 8, 9, 10, 11, 12);

#include <SFEMP3Shield.h>


/**
 * \brief Object instancing the SdFat library.
 *
 * principal object for handling all SdCard functions.
 */
SdFat sd;

/**
 * \brief Object instancing the SFEMP3Shield library.
 *
 * principal object for handling all the attributes, members and functions for the library.
 */
SFEMP3Shield MP3player;


//------------------------------------------------------------------------------
/**
 * \brief Setup the Arduino Chip's feature for our use.
 *
 * After Arduino's kernel has booted initialize basic features for this
 * application, such as Serial port and MP3player objects with .begin.
 * Along with displaying the Help Menu.
 *
 * \note returned Error codes are typically passed up from MP3player.
 * Whicn in turns creates and initializes the SdCard objects.
 *
 * \see
 * \ref Error_Codes
 */

void setup()
{
    Serial.begin(115200);
    Serial.print(F("Free RAM = ")); // available in Version 1.0 F() bases the string to into Flash, to use less SRAM.
    Serial.print(FreeRam(), DEC);  // FreeRam() is provided by SdFatUtil.h
    Serial.println(F(" Should be a base line of 1029, on ATmega328 when using INTx"));
    Serial.print(F("SD_SEL:")); Serial.println(SD_SEL);
   // Serial.flush();

#if 1
    Wire.begin();
    reader.init();
    writer.init();
    
    Serial.println(F("x1"));
    writer.digitalWrite( outputWire_ramChipSelect, HIGH );
    writer.digitalWrite( outputWire_vs1063controlChipSelect, HIGH );
    writer.digitalWrite( outputWire_vs1063dataChipSelect, HIGH );
    writer.digitalWrite( outputWire_attinyScreenDriverChipSelect, HIGH );
    // sdcard library needs this apparently
    pinMode( 10, OUTPUT );
    digitalWrite( 10, HIGH );
    // setup pins for vs1063
    pinMode(MP3_DREQ,  INPUT  );
    pinMode(MP3_XCS,   OUTPUT );
    pinMode(MP3_XDCS,  OUTPUT );
    pinMode(MP3_RESET, OUTPUT );
    MP3player.cs_high();
    MP3player.dcs_high();
    digitalWrite(MP3_RESET, HIGH);


    Serial.println(F("x1"));
    SPI.begin();
 //   SPI.setDataMode(SPI_MODE0);
  //  SPI.setClockDivider(SPI_CLOCK_DIV16);
  //  SPI.setBitOrder( MSBFIRST );
    
    Serial.println(F("waiting"));
    delay(1000);
    Serial.println(F("starting screen..."));

    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Welcome to Playa");
    Serial.println("have screen...");
    delay(100);

    Serial.println("starting sdcard...");
    //Initialize the SdCard.
//    if(!sd.begin(SD_SEL, SPI_HALF_SPEED))
    if(!sd.begin(SD_SEL, SPI_FULL_SPEED))
    {
        lcd.setCursor(0, 1);
        lcd.print("sdcard error.");
        delay(200);
        sd.initErrorHalt();
    }
    if(!sd.chdir("/")) sd.errorHalt("sd.chdir");
    
    lcd.setCursor(0, 1);
    lcd.print("version 0.1");
    
    uint8_t result; //result code from some function as to be tested at later time.

    Serial.println(F("starting vs1063..."));
    delay(200);
    //Initialize the MP3 Player Shield
    result = MP3player.begin();
    //check result, see readme for error codes.
    if(result != 0) {
        Serial.print(F("Error code: "));
        Serial.print(result);
        Serial.println(F(" when trying to start MP3 player"));
        if( result == 6 ) {
            Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
            Serial.println(F("Use the \"d\" command to verify SdCard can be read")); // can be removed for space, if needed.
        }
    }

    lcd.print("..OK");
    MP3player.setDisplay( &lcd );

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( MP3player.getPlaylist().playlistname );

    Serial.print(F("setup done."));
#endif
    
}

//------------------------------------------------------------------------------
/**
 * \brief Main Loop the Arduino Chip
 *
 * This is called at the end of Arduino kernel's main loop before recycling.
 * And is where the user's serial input of bytes are read and analyzed by
 * parsed_menu.
 *
 * Additionally, if the means of refilling is not interrupt based then the
 * MP3player object is serviced with the availaible function.
 *
 * \note Actual examples of the libraries public functions are implemented in
 * the parse_menu() function.
 */
void loop()
{
    #if 1

    
    byte b = reader.poll();  
//    Serial << b << endl;
    bPlayPause.poll();
    bNextTrack.poll();
    bTemp.poll();
    if( bTemp.released() )
    {
        Serial << F("temp button release!\n");
        MP3player.nextPlaylistCircular();
//        MP3player.showNormalDisplay();
    }
    if( bTemp.held() )
        Serial << F("temp button held down....") << endl;
        
    if( bPlayPause.released() )
    {
        Serial << F("play pause released") << endl;
        MP3player.togglePlayPause();
    }
    

    if( bNextTrack.released() )
    {
        Serial << F("bnext track pressed...") << endl;
        MP3player.nextTrackCircular();
    }

    timer.run();
    
    if( MP3player.getFinishedPlayingSong() )
    {
        MP3player.getPlaylist().nextTrack();
    }
    
        
// Below is only needed if not interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) \
    && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
    ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )

  MP3player.available();
#endif
#endif
}

//------------------------------------------------------------------------------
/**
 * \brief Decode the Menu.
 *
 * Parses through the characters of the users input, executing corresponding
 * MP3player library functions and features then displaying a brief menu and
 * prompting for next input command.
 */
void parse_menu(byte key_command) {

  uint8_t result; // result code from some function as to be tested at later time.

  // Note these buffer may be desired to exist globably.
  // but do take much space if only needed temporarily, hence they are here.
  char title[30]; // buffer to contain the extract the Title from the current filehandles
  char artist[30]; // buffer to contain the extract the artist name from the current filehandles
  char album[30]; // buffer to contain the extract the album name from the current filehandles

  Serial.print(F("Received command: "));
  Serial.write(key_command);
  Serial.println(F(" "));

  //if s, stop the current track
  if(key_command == 's') {
    Serial.println(F("Stopping"));
    MP3player.stopTrack();

  //if 1-9, play corresponding track
  } else if(key_command >= '1' && key_command <= '9') {
    //convert ascii numbers to real numbers
    key_command = key_command - 48;

#if USE_MULTIPLE_CARDS
    sd.chvol(); // assign desired sdcard's volume.
#endif

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Loading...");
            lcd.setCursor(0, 1);
//            lcd.print(title);

    //tell the MP3 Shield to play a track
    result = MP3player.playTrack(key_command);

    //check result, see readme for error codes.
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    } else {

      Serial.println(F("Playing by number(x):"));
    }


  //if < or > to change Play Speed
  } else if((key_command == '>') || (key_command == '<')) {
    uint16_t playspeed = MP3player.getPlaySpeed(); // create key_command existing variable
    // note playspeed of Zero is equal to ONE, normal speed.
    if(key_command == '>') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if(playspeed >= 254) { // range check
        playspeed = 5;
      } else {
        playspeed += 1; // keep it simpler with whole dB's
      }
    } else {
      if(playspeed == 0) { // range check
        playspeed = 0;
      } else {
        playspeed -= 1;
      }
    }
    MP3player.setPlaySpeed(playspeed); // commit new playspeed
    Serial.print(F("playspeed to "));
    Serial.println(playspeed, DEC);

  /* Alterativly, you could call a track by it's file name by using playMP3(filename);
  But you must stick to 8.1 filenames, only 8 characters long, and 3 for the extension */
  } else if(key_command == 'f' || key_command == 'F') {
    uint32_t offset = 0;
    if (key_command == 'F') {
      offset = 2000;
    }

    //create a string with the filename
    char trackName[] = "track001.mp3";

#if USE_MULTIPLE_CARDS
    sd.chvol(); // assign desired sdcard's volume.
#endif
    //tell the MP3 Shield to play that file
    result = MP3player.playMP3(trackName, offset);
    //check result, see readme for error codes.
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    }

  /* Display the file on the SdCard */
  } else if(key_command == 'd') {
    if(!MP3player.isPlaying()) {
      // prevent root.ls when playing, something locks the dump. but keeps playing.
      // yes, I have tried another unique instance with same results.
      // something about SdFat and its 500byte cache.
      Serial.println(F("Files found (name date time size):"));
      sd.ls(LS_R | LS_DATE | LS_SIZE);
    } else {
      Serial.println(F("Busy Playing Files, try again later."));
    }


  /* Get and Display the Audio Information */
  } else if(key_command == 'i') {
    MP3player.getAudioInfo();

  } else if(key_command == 'p') {
    if( MP3player.getState() == playback) {
      MP3player.pauseMusic();
      Serial.println(F("Pausing"));
    } else if( MP3player.getState() == paused_playback) {
      MP3player.resumeMusic();
      Serial.println(F("Resuming"));
    } else {
      Serial.println(F("Not Playing!"));
    }

  } else if(key_command == 'r') {
    MP3player.resumeMusic(2000);

  } else if(key_command == 'R') {
    MP3player.stopTrack();
    MP3player.vs_init();
    Serial.println(F("Reseting VS10xx chip"));

  } else if(key_command == 'e') {
    uint8_t earspeaker = MP3player.getEarSpeaker();
    if(earspeaker >= 3){
      earspeaker = 0;
    } else {
      earspeaker++;
    }
    MP3player.setEarSpeaker(earspeaker); // commit new earspeaker
    Serial.print(F("earspeaker to "));
    Serial.println(earspeaker, DEC);


  } else if(key_command == 'g') {
    int32_t offset_ms = 20000; // Note this is just an example, try your own number.
    Serial.print(F("jumping to "));
    Serial.print(offset_ms, DEC);
    Serial.println(F("[milliseconds]"));
    result = MP3player.skipTo(offset_ms);
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to skip track"));
    }

  } else if(key_command == 'k') {
    int32_t offset_ms = -1000; // Note this is just an example, try your own number.
    Serial.print(F("moving = "));
    Serial.print(offset_ms, DEC);
    Serial.println(F("[milliseconds]"));
    result = MP3player.skip(offset_ms);
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to skip track"));
    }

  } else if(key_command == 'O') {
    MP3player.end();
    Serial.println(F("VS10xx placed into low power reset mode."));

  } else if(key_command == 'o') {
    MP3player.begin();
    Serial.println(F("VS10xx restored from low power reset mode."));

  } else if(key_command == 'S') {
    Serial.println(F("Current State of VS10xx is."));
    Serial.print(F("isPlaying() = "));
    Serial.println(MP3player.isPlaying());

    Serial.print(F("getState() = "));
    switch (MP3player.getState()) {
    case uninitialized:
      Serial.print(F("uninitialized"));
      break;
    case initialized:
      Serial.print(F("initialized"));
      break;
    case deactivated:
      Serial.print(F("deactivated"));
      break;
    case loading:
      Serial.print(F("loading"));
      break;
    case ready:
      Serial.print(F("ready"));
      break;
    case playback:
      Serial.print(F("playback"));
      break;
    case paused_playback:
      Serial.print(F("paused_playback"));
      break;
    case testing_memory:
      Serial.print(F("testing_memory"));
      break;
    case testing_sinewave:
      Serial.print(F("testing_sinewave"));
      break;
    }
    Serial.println();


  }

  // print prompt after key stroke has been processed.
  Serial.println(F("Enter 1-9,f,F,s,d,+,-,i,>,<,p,r,R,t,m,M,g,k,h,O,o,D,S,V,l,01-65534 :"));
}


void SerialPrintPaddedNumber(int16_t value, int8_t digits ) {
  int currentMax = 10;
  for (byte i=1; i<digits; i++){
    if (value < currentMax) {
      Serial.print("0");
    }
    currentMax *= 10;
  }
  Serial.print(value);
}
