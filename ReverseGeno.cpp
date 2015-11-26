/*
  Reverse Geno
  Copyright (C) 2015, Adam Sinck (the map)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <memory>
#include <sstream>
#include <stdexcept>


#include "bzfsAPI.h"
#include "plugin_utils.h"


// Define plug-in name
const std::string PLUGIN_NAME = "Reverse Geno";


static void genoOtherTeam(bz_eTeamType team = eNoTeam, bool spawnOnBase = false, int killerID = -1, int bait = -1, const char* flagAbbr = NULL)
{
    // Create a list of players
    std::shared_ptr<bz_APIIntList> playerList(bz_getPlayerIndexList());

    // If the playerlist is valid
    if (playerList)
    {
        //if no team, kill everyone
        if (team == eNoTeam && bait != -1 && bait == killerID)
        {
            for (unsigned int i = 0; i < playerList->size(); i++)
            {
                int playerID = playerList->get(i);
                bz_killPlayer(playerID, spawnOnBase, killerID, flagAbbr);
            }
        }

        else if (bait != -1 && bait == killerID)
        {
            for (unsigned int i = 0; i < playerList->size(); i++)
            {
                int playerID = playerList->get(i);
                if (bz_getPlayerTeam(playerID) == team)
                    bz_killPlayer(playerID, spawnOnBase, killerID, flagAbbr);
            }
        }
            
    }
}

class ReverseGeno : public bz_Plugin
{
public:
    virtual const char* Name () {return "ReverseGeno";}
    virtual void Init (const char* config);
    virtual void Event (bz_EventData *eventData);
    virtual void Cleanup (void);

    int     bait;          ///< The ID of the player holding the flag
    int     fish;          ///< The ID of the player to shoot the bait
    int     messageNumber; ///< The number of the message that the server is giving to the user
                           ///<     to tell them how long till death

    double  grabTime;      ///< The server time of when the flag was picked up
    double  shotTime;      ///< The server time when the bait was shot
};

BZ_PLUGIN(ReverseGeno)


void ReverseGeno::Init (const char* /*commandLine*/)
{
    // Register our events with Register()
    Register(bz_eFlagGrabbedEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_eTickEvent);

    // Register our custom flags
    bz_RegisterCustomFlag("RG", "Reverse Geno", "Get shot within five seconds to kill the other team and avoid self-destruction.", 0, eBadFlag);

    bait = -1;
    fish = -1;
    grabTime = -1;
    shotTime = -1;
}

void ReverseGeno::Cleanup (void)
{
    Flush(); // Clean up all the events
}

void ReverseGeno::Event (bz_EventData *eventData)
{
    switch (eventData->eventType)
    {
    case bz_eFlagGrabbedEvent: // This event is called each time a flag is grabbed by a player
        {
            bz_FlagGrabbedEventData_V1* flagGrabData = (bz_FlagGrabbedEventData_V1*)eventData;
            if (strcmp(flagGrabData->flagType, "RG") == 0)
            {
                bait = flagGrabData->playerID;
                grabTime = flagGrabData->eventTime;
                messageNumber = 5;
                bz_sendTextMessagef(BZ_SERVER, bait, "FIVE SECONDS TO DIE! Get killed before you DIE!");
            }

            // Data
            // ---
            //    (int)           playerID  - The player that grabbed the flag
            //    (int)           flagID    - The flag ID that was grabbed
            //    (bz_ApiString)  flagType  - The flag abbreviation of the flag that was grabbed
            //    (float[3])      pos       - The position at which the flag was grabbed
            //    (double)        eventTime - This value is the local server time of the event.
        }
        break;


    case bz_ePlayerDieEvent: // This event is called each time a tank is killed.
        {
            bz_PlayerDieEventData_V1* dieData = (bz_PlayerDieEventData_V1*)eventData;
            if (bait != -1 && bait != dieData->killerID && bait == dieData->playerID)
            {
                //bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Bait: %s, Player: %s (%d,%d)", bz_getPlayerCallsign(bait), bz_getPlayerCallsign(dieData->playerID), bait, dieData->playerID);
                if (bz_getPlayerTeam(bait) == bz_getPlayerTeam(dieData->killerID)) {
                    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Fail! %s just committed genocide on their own team!", bz_getPlayerCallsign(bait));
                    //bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Bait: %s, other: %s ", bz_getPlayerTeam(bait), bz_getPlayerTeam(dieData->playerID));
                }
                else {
                    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Score! %s just baited %s into committing genocide on their own team!", bz_getPlayerCallsign(bait), bz_getPlayerCallsign(dieData->killerID));
                }
                
                genoOtherTeam(dieData->killerTeam, false, dieData->playerID, bait, "RG");
                grabTime = -1;
                bait = -1;
                //bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Bait: %0d, Player: %s (%d,%d)", bait, bz_getPlayerCallsign(dieData->playerID), bait, dieData->playerID);
            }
            else if (bait != -1 && bait == dieData->killerID && bait == dieData->playerID)
            {
                grabTime = -1;
                bait = -1;
            }

            // Data
            // ---
            //   (int)                   playerID       - ID of the player who was killed.
            //   (bz_eTeamType)          team           - The team the killed player was on.
            //   (int)                   killerID       - The owner of the shot that killed the player, or BZ_SERVER for server side kills
            //   (bz_eTeamType)          killerTeam     - The team the owner of the shot was on.
            //   (bz_ApiString)          flagKilledWith - The flag name the owner of the shot had when the shot was fired.
            //   (int)                   shotID         - The shot ID that killed the player, if the player was not killed by a shot, the id will be -1.
            //   (bz_PlayerUpdateState)  state          - The state record for the killed player at the time of the event
            //   (double)                eventTime      - Time of the event on the server.
        }
        break;

    case bz_eTickEvent: // This event is called once for each BZFS main loop
        {
            bz_TickEventData_V1* tickData = (bz_TickEventData_V1*)eventData;
            if (grabTime != -1) {
                double timeSinceGrab = tickData->eventTime - grabTime;
                if (timeSinceGrab >= 5)
                {
                    int player = bait;
                    bait = -1;
                    grabTime = -1;
                    messageNumber = 5;
                    bz_sendTextMessagef(BZ_SERVER, player, "YOU FAILED TO GET KILLED, SO YOU DIE.");
                    bz_killPlayer(player, 0, BZ_SERVER, "RG");
                }
                else if (timeSinceGrab >= 4 && messageNumber > 1)
                {
                    bz_sendTextMessagef(BZ_SERVER, bait, "ONE");
                    messageNumber = 1;
                }
                else if (timeSinceGrab >= 3 && messageNumber > 2)
                {
                    bz_sendTextMessagef(BZ_SERVER, bait, "TWO");
                    messageNumber = 2;
                }
                else if (timeSinceGrab >= 2 && messageNumber > 3)
                {
                    bz_sendTextMessagef(BZ_SERVER, bait, "THREE");
                    messageNumber = 3;
                }
                else if (timeSinceGrab >= 1 && messageNumber > 4)
                {
                    bz_sendTextMessagef(BZ_SERVER, bait, "FOUR");
                    messageNumber = 4;
                }
            }
            
            // Data
            // ---
            //    (double)  eventTime - Local Server time of the event (in seconds)
        }
        break;

    default: break;
    }
}
