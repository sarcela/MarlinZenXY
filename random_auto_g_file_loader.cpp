/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


// NOTES:
//  1) couldn't figure out how to save & retrieve integers from
//     SD card so converted them to/from ASCII sequences.
//  2) standard RAND_MAX is 32 bit but was too lazy to do 10 digit ASCII
//     conversions so went with 16 bit values for RAND_MAX and the seed.
//  3) openAndPrintFile wouldn't open files with lower case names to I
//     had to spin my own version
//  4) Can have only one file open at a time.
//  5) Made this re-entrant so that the main Marlin loop could execute
//     otherwise the injected commands never get executed.
//  6) Need to add something like the following to the main loop in
//     MarlinCore.cpp to automatically execute while still allowing 
//     enough time to process commands.
//
//   bool random_auto_g_file_loader();              // add anywhere before the loop()
//   millis_t millis_auto_g;                        // add anywhere before the loop()
//   void loop() {                                  // existing code
//       do {                                       // existing code
//         if (millis() - millis_auto_g > 5000) {   // put this inside the do loop
//           random_auto_g_file_loader();           // put this inside the do loop
//           millis_auto_g = millis();              // put this inside the do loop
//         }


#include "../module/motion.h"
#include "../sd/cardreader.h"
#include "../module/settings.h"
//include "../feature/powerloss.h"  // Marlin 2.1.2 has problems with POWER_LOSS_RECOVERY
#include "../libs/numtostr.h"
#include <stdlib.h>
#include "../gcode/queue.h"

#if ENABLED(SDSUPPORT)

uint8_t get_max_autofile();
uint8_t get_max_ERRASER();

 
bool random_auto_g_file_loader() {

extern int lastrandomNumber;                          // last randon number used 
//extern int DwPot; // re-entrant version
extern bool DWcont;
  static uint8_t auto_g_state = 0;
  static uint8_t alternator = 0;
  static uint8_t max_autofile_index;
  static uint8_t max_autofile_index_ERRASER;

  const char fname[] = "seed.txt";   // get the last seed from the SD card
  uint16_t seed_16;
  char autoname[18];
  int randomNumber;

  switch(auto_g_state) {
    
    case 0:   // start
      //SERIAL_ECHOLNPGM("start");
      if (ENABLED(SDCARD_EEPROM_EMULATION))
        settings.first_load();
      card.cdroot();
      max_autofile_index = get_max_autofile();  // also mounts the card
      //SERIAL_ECHOLNPGM("max_autofile_index: ", max_autofile_index);
      max_autofile_index_ERRASER = get_max_ERRASER();

      card.openFileRead(fname);
      if (!card.isFileOpen()) {
        SERIAL_ECHOLNPGM("Failed to open ", fname, " to read.");
        return false;
      }

      uint8_t buf[5];
      if (COUNT(buf) != card.read(buf, COUNT(buf)))
          SERIAL_ECHOLNPGM("Failed to read from file.");
      //SERIAL_ECHOLNPGM("buf[0]  ", buf[0] - '0', "  buf[1]  ", buf[1] - '0', "  buf[2]  ", buf[2] - '0', "  buf[3]  ", buf[3] - '0', "  buf[4]  ", buf[4] - '0');
      seed_16 =  (buf[0] - '0')*10000 +(buf[1] - '0')*1000 + (buf[2] - '0')*100 + (buf[3] - '0')*10 + (buf[4] - '0') ;  // convert from ascii to int
      //SERIAL_ECHOLNPGM("seed_16 from SD card: ", seed_16);

      srand(seed_16+(millis()&0xFFFF));
      seed_16 = rand();

      #define DIGIT(n) ('0' + (n))
      #define DIGIMOD(n, f) DIGIT((n)/(f) % 10)
      buf[0] = DIGIMOD(seed_16, 10000);  // convert to ASCII bytes
      buf[1] = DIGIMOD(seed_16, 1000);
      buf[2] = DIGIMOD(seed_16, 100);
      buf[3] = DIGIMOD(seed_16, 10);
      buf[4] = DIGIMOD(seed_16, 1);
      //SERIAL_ECHOLNPGM("buf[0]  ", buf[0] - '0', "  buf[1]  ", buf[1] - '0', "  buf[2]  ", buf[2] - '0', "  buf[3]  ", buf[3] - '0', "  buf[4]  ", buf[4] - '0');

      card.openFileWrite(fname);
      card.write(buf, COUNT(buf));
      card.closefile();

      safe_delay(5000);

      sprintf_P(autoname, PSTR("M203 X%s Y%s\n"), i16tostr3left(MAXFEEDRATE), i16tostr3left(MAXFEEDRATE));
      SERIAL_ECHOLNPGM("Loading FD: ", autoname);
      queue.inject(autoname);

      queue.inject(F("M23 AUTO0.G\nM24)"));

      auto_g_state = 1;
      // fall through to case 1
      break;
    case 1:  // generate the next random number and read file
          // Don't run auto#.g when a PLR file exists
          // if (card.isMounted() && TERN1(POWER_LOSS_RECOVERY, !recovery.valid())) {   // Marlin 2.1.2 has problems with POWER_LOSS_RECOVERY

          // Generate a random number between 1 and max_autofile_index;

          #define RAND_MIN 1    // lowest number of desired range
          #undef RAND_MAX               // standard RAND_MAX is 32 bit
          #define RAND_MAX 0xFFFF

          switch(alternator) {
            case 0:
              if (DWcont){
                if (!card.isPrinting()) {
                  seed_16 = rand();
                  //SERIAL_ECHOLNPGM("final seed_16: ", seed_16);

                  int count =0;
                  do {
                    randomNumber = (seed_16 % max_autofile_index_ERRASER)+RAND_MIN;
                    //SERIAL_ECHOLNPGM("randomNumber: ", randomNumber);
                    count++;
                    if (count > 5) {
                      randomNumber = (lastrandomNumber+1)%max_autofile_index_ERRASER;
                      lastrandomNumber = 0;
                    }
                  } while (randomNumber==lastrandomNumber);
                  lastrandomNumber = randomNumber;

                  //SERIAL_ECHOLNPGM("DWcont: ", DWcont);


                  sprintf_P(autoname, PSTR("M23 ERRASER%s.G\nM24"), i16tostr3left(randomNumber));
                  //SERIAL_ECHOLNPGM("autoname: ", autoname);
                  queue.inject(autoname);
                  alternator = 1;
                  DWcont = false;
                  //SERIAL_ECHOLNPGM("DWcont: ", DWcont);
                  break;
                }
              }
          case 1:
            //if (DWcont){
              if (!card.isPrinting()) {
                  seed_16 = rand();
                  //SERIAL_ECHOLNPGM("final seed_16: ", seed_16);

                  int count =0;
                  do {
                    randomNumber = (seed_16 % max_autofile_index)+RAND_MIN;
                    //SERIAL_ECHOLNPGM("randomNumber: ", randomNumber);
                    count++;
                    if (count > 5) {
                      randomNumber = (lastrandomNumber+1)%max_autofile_index;
                      lastrandomNumber = 0;
                   }
                  } while (randomNumber==lastrandomNumber);
                  lastrandomNumber = randomNumber;

                  sprintf_P(autoname, PSTR("M23 PATTERN%s.G\nM24"), i16tostr3left(randomNumber));
                  //SERIAL_ECHOLNPGM("autoname: ", autoname);
                  queue.inject(autoname);
                  alternator = 0;
                  break;
                }
            //}
          }          
          

        break;
  }
  //SERIAL_ECHOLNPGM("end of M220");
  return true;
}

uint8_t get_max_autofile() {

  if (!card.isMounted())
    card.mount();
  else if (ENABLED(SDCARD_EEPROM_EMULATION))
    settings.first_load();
  uint8_t counter = 1;
  
  // Don't run auto#.g when a PLR file exists
  if (card.isMounted() && TERN1(POWER_LOSS_RECOVERY, !recovery.valid())) {
    char autoname[15];
    do {
      sprintf_P(autoname, PSTR("PATTERN%s.g"), i16tostr3left(counter));
       //SERIAL_ECHOLNPGM("file name: ", autoname, "  exits: ", card.fileExists(autoname));
      counter++;
    } while(card.fileExists(autoname));
    
  }

  return (counter - 2);
}

uint8_t get_max_ERRASER() {

  if (!card.isMounted())
    card.mount();
  else if (ENABLED(SDCARD_EEPROM_EMULATION))
    settings.first_load();
  uint8_t counterERRASER = 1;

  // Don't run auto#.g when a PLR file exists
  if (card.isMounted() && TERN1(POWER_LOSS_RECOVERY, !recovery.valid())) {
    char autoname[15];
    do {
      sprintf_P(autoname, PSTR("ERRASER%s.g"), i16tostr3left(counterERRASER));
       //SERIAL_ECHOLNPGM("file name: ", autoname, "  exits: ", card.fileExists(autoname));
      counterERRASER++;
    } while(card.fileExists(autoname));

  }

  return (counterERRASER - 2);
}
#endif
