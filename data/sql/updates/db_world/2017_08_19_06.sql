-- DB update 2017_08_19_05 -> 2017_08_19_06
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2017_08_19_05';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2017_08_19_05 2017_08_19_06 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1493321428049355400'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--
INSERT INTO version_db_world (`sql_rev`) VALUES ('1493321428049355400');
-- Tranquillien RP event
SET @AURIFEROUS :=16231;
SET @SCRIPT := 1623100;
SET @MALTENDIS :=16251;
SET @MAVREN := 16252;
SET @VALWYN := 16289;

UPDATE `creature_template` SET `AIName`="SmartAI" WHERE `entry` IN (@AURIFEROUS, @MAVREN, @VALWYN, @MALTENDIS);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (@AURIFEROUS, @SCRIPT, @MAVREN, @VALWYN, @MALTENDIS);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@AURIFEROUS,0,  0, 0, 1, 0, 100, 0, 10000, 60000, 580000, 620000, 80, @SCRIPT, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Aureous - Out Of Combat - Run Script"),
(@SCRIPT,    9,  0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, @VALWYN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 0 (Advisor Valwyn)"),
(@SCRIPT,    9,  1, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 0"),
(@SCRIPT,    9,  2, 0, 0, 0, 100, 0, 60000, 70000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Face High Executor Mavren"),
(@SCRIPT,    9,  3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 1"),
(@SCRIPT,    9,  4, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 1 1 (High Executor Mavren)"),
(@SCRIPT,    9,  5, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, @MALTENDIS, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 0 (Deathstalker Maltendis)"),
(@SCRIPT,    9,  6, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Reset Orientation"),
(@SCRIPT,    9,  7, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 0 (High Executor Mavren)"),
(@SCRIPT,    9,  8, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 19, @VALWYN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 1 (Advisor Valwyn)"),
(@SCRIPT,    9,  9, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 45, 2, 2, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 2 2 (High Executor Mavren)"),
(@SCRIPT,    9, 10, 0, 0, 0, 100, 0, 7000, 7000, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 2"),
(@SCRIPT,    9, 11, 0, 0, 0, 100, 0, 50000, 70000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Face High Executor Mavren"),
(@SCRIPT,    9, 12, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 3"),
(@SCRIPT,    9, 13, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 1 1 (High Executor Mavren)"),
(@SCRIPT,    9, 14, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, @MALTENDIS, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 0 (Deathstalker Maltendis)"),
(@SCRIPT,    9, 15, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Reset Orientation"),
(@SCRIPT,    9, 16, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 1 (High Executor Mavren)"),
(@SCRIPT,    9, 17, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 19, @VALWYN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 1 (Advisor Valwyn)"),
(@SCRIPT,    9, 18, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 45, 2, 2, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 2 2 (High Executor Mavren"),
(@SCRIPT,    9, 19, 0, 0, 0, 100, 0, 7000, 7000, 0, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 4"),
(@SCRIPT,    9, 20, 0, 0, 0, 100, 0, 50000, 70000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Face High Executor Mavren"),
(@SCRIPT,    9, 21, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 5"),
(@SCRIPT,    9, 22, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 1 1 (High Executor Mavren)"),
(@SCRIPT,    9, 23, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, @MALTENDIS, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 0 (Deathstalker Maltendis)"),
(@SCRIPT,    9, 24, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 66, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Reset Orientation"),
(@SCRIPT,    9, 25, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 1, 2, 0, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 2 (High Executor Mavren)"),
(@SCRIPT,    9, 26, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 19, @VALWYN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 1 (Advisor Valwyn)"),
(@SCRIPT,    9, 27, 0, 0, 0, 100, 0, 4000, 4000, 0, 0, 45, 2, 2, 0, 0, 0, 0, 19, @MAVREN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 2 2 (High Executor Mavren"),
(@SCRIPT,    9, 28, 0, 0, 0, 100, 0, 7000, 7000, 0, 0, 1, 6, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 6"),
(@SCRIPT,    9, 29, 0, 0, 0, 100, 0, 150000, 190000, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, @MALTENDIS, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 1 1 (Deathstalker Maltendis)"),
(@SCRIPT,    9, 30, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 19, @MALTENDIS, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 1 (Deathstalker Maltendis)"),
(@SCRIPT,    9, 31, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, @VALWYN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 1 1 (Advisor Valwyn)"),
(@SCRIPT,    9, 32, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 1, 2, 0, 0, 0, 0, 0, 19, @VALWYN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Talk 2 (Advisor Valwyn)"),
(@SCRIPT,    9, 33, 0, 0, 0, 100, 0, 0, 0, 0, 0, 45, 2, 2, 0, 0, 0, 0, 19, @MALTENDIS, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 2 2 (Deathstalker Maltendis)"),
(@SCRIPT,    9, 34, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 45, 2, 2, 0, 0, 0, 0, 19, @VALWYN, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 2 2 (Advisor Valwyn)"),
(@SCRIPT,    9, 35, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 45, 3, 3, 0, 0, 0, 0, 19, @MALTENDIS, 0, 0, 0, 0, 0, 0, "Dame Auriferous - On Script - Set Data 3 3 (Deathstalker Maltendis)"),
(@VALWYN,    0, 0, 0, 38, 0, 100, 0, 1, 1, 1000, 1000, 66, 0, 0, 0, 0, 0, 0, 19, @MALTENDIS, 0, 0, 0, 0, 0, 0, "Advisor Valwyn - On Data 1 1 Set - Face Deathstalker Maltendis"),
(@VALWYN,    0, 1, 0, 38, 0, 100, 0, 2, 2, 1000, 1000, 66, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Advisor Valwyn - On Data 2 2 Set - Reset Orientation"),
(@MAVREN,    0, 0, 0, 38, 0, 100, 0, 1, 1, 1000, 1000, 66, 0, 0, 0, 0, 0, 0, 19, 16231, 0, 0, 0, 0, 0, 0, "High Executor Mavren - On Data 1 1 Set - Face Dame Auriferious"),
(@MAVREN,    0, 1, 0, 38, 0, 100, 0, 2, 2, 1000, 1000, 66, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "High Executor Mavren - On Data 2 2 Set - Reset Orientation"),
(@MALTENDIS, 0, 0, 0, 38, 0, 100, 0, 1, 1, 1000, 1000, 66, 0, 0, 0, 0, 0, 0, 19, @VALWYN, 0, 0, 0, 0, 0, 0, "Deathstalker Maltendis - On Data 1 1 Set - Face Advisor Valwyn"),
(@MALTENDIS, 0, 1, 0, 38, 0, 100, 0, 2, 2, 1000, 1000, 5, 153, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Deathstalker Maltendis - On Data 2 2 Set - Emote Laugh"),
(@MALTENDIS, 0, 2, 0, 38, 0, 100, 0, 3, 3, 1000, 1000, 66, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Deathstalker Maltendis - On Data 3 3 Set - Reset Orientation");

DELETE FROM `creature_text` WHERE `entry` IN (@AURIFEROUS, @MAVREN, @VALWYN, @MALTENDIS);
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(@AURIFEROUS, 0, 0, "%s nods her head yes at what her advisor has to say.", 16, 0, 100, 273, 0, 0, 13362, 0, "Dame Auriferous"),
(@AURIFEROUS, 0, 1, "%s disagrees with whatever it is that Valwyn has whispered to her.", 16, 0, 100, 274, 0, 0, 13363, 0, "Dame Auriferous"),
(@AURIFEROUS, 0, 2, "%s seems nonplussed by whatever it is that her advisor has whispered in her ear.", 16, 0, 100, 6, 0, 0, 13365, 0, "Dame Auriferous"),
(@AURIFEROUS, 1, 0, "Executor, your direct assault approach will cost us more lives than I am comfortable with. A more nuanced strategy is called for; one that involves us reclaiming the surrounding territory, starting with the villages so as to cut off all points of reinforcement to Deatholme.", 12, 1, 100, 0, 0, 0, 12552, 0, "Dame Auriferous"),
(@AURIFEROUS, 2, 0, "I hear what you are saying, Mavren. Nevertheless, the final decision is mine. I appreciate your assistance, but the majority of the lives on the line are blood elf. I will not have those lives carelessly thrown away!", 12, 1, 100, 0, 0, 0, 12571, 0, "Dame Auriferous"),
(@AURIFEROUS, 3, 0, "How long until Forsaken reinforcements arrive? Our position here is tenuous. Your Lady promised us more soldiers. We must be able to concentrate solely on Deatholme and the Scourge!", 12, 1, 100, 0, 0, 0, 12557, 0, "Dame Auriferous"),
(@AURIFEROUS, 4, 0, "Mavren, I don't want excuses, I want results! The Farstriders aren't available and we've received all that we're going to get from Silvermoon for now. I cannot ignore the Shadowpine trolls and Zul'Aman... they're arrayed across our eastern border!", 12, 1, 100, 0, 0, 0, 12572, 0, "Dame Auriferous"),
(@AURIFEROUS, 5, 0, "No, I was right to begin with. Deatholme must come last. We must secure all of the Ghostlands first. I will not commit the forces here to a battle against Dar'khan with enemies to our flank and rear!", 12, 1, 100, 0, 0, 0, 12562, 0, "Dame Auriferous"),
(@AURIFEROUS, 6, 0, "Disagree with me all you like, High Executor. I will weigh your counsel, and then we will take the steps to free all of Quel'Thalas as I deem necessary. We shall continue this discussion anon.", 12, 1, 100, 0, 0, 0, 12573, 0, "Dame Auriferous"),
(@MAVREN,     0, 0, "With all due respect, milady, you have not fought the Scourge as I have. The Lady appointed me to assist you in defeating Dar'khan because of that, and you would do well to consider what I have to say.", 12, 1, 100, 0, 0, 0, 12553, 0, "High Executor Mavren"),
(@MAVREN,     0, 1, "I strongly disagree. Dar'khan is not going to get reinforcements because he'll be bottlenecked. Deatholme has only one way in and out. Your traitor was a fool to corner himself there, and I intend to exploit that weakness!", 12, 1, 100, 0, 0, 0, 12554, 0, "High Executor Mavren"),
(@MAVREN,     0, 2, "This is war, Dame Auriferous, and in any war lives will be lost! The only areas that we need to secure are the two ziggurats. We will turtle in and take the Tower of the Damned and its master by brute force. Then you will have your Quel'Thalas.", 12, 1, 100, 0, 0, 0, 12555, 0, "High Executor Mavren"),
(@MAVREN,     0, 3, "And if your Farstriders were here to reinforce us, I might agree. But, they are busying themselves instead with the Shadowpine trolls on your eastern border. The longer we wait for them, the stronger the Scourge will become.", 12, 1, 100, 0, 0, 0, 12556, 0, "High Executor Mavren"),
(@MAVREN,     1, 0, "I believe that you're making my argument for me, but yes it would be nice if we had more reinforcements. I would counter that it would be faster to pull the Farstriders from their senseless skirmishing with the trolls.", 12, 1, 100, 0, 0, 0, 12558, 0, "High Executor Mavren"),
(@MAVREN,     1, 1, "Dame Auriferous, we are the reinforcements. It may come to pass that The Lady will send more men in time. Better that you convince Silvermoon City to muster all of its forces and put them at our disposal here.", 12, 1, 100, 0, 0, 0, 12559, 0, "High Executor Mavren"),
(@MAVREN,     1, 2, "You are correct; we must concentrate on the Scourge. Ignore the trolls and the villages, and focus our attentions on Deatholme! With Dar'khan defeated you will find that the rest of the Scourge 'body' will fall quickly, lacking their 'head'.", 12, 1, 100, 0, 0, 0, 12560, 0, "High Executor Mavren"),
(@MAVREN,     1, 3, "Forsaken reinforcements? Undercity is stretched thin with the Scourge on all sides. No, this will be a matter largely dealt with by the blood elves. We Forsaken are here as backup and advisors.", 12, 1, 100, 0, 0, 0, 12561, 0, "High Executor Mavren"),
(@MAVREN,     2, 0, "Again I strongly disagree with your view of the strategic situation. Your plan will spread our forces too thin. It will leave us with only a small force to assault Deatholme. I cannot in good conscience execute such a strategy.", 12, 1, 100, 0, 0, 0, 12567, 0, "High Executor Mavren"),
(@MAVREN,     2, 1, "Spread throughout the Ghostlands, our forces will be of little use in an assault on Deatholme. No, milady, I am here to kill Dar'khan and that is what I intend to do!", 12, 1, 100, 0, 0, 0, 12568, 0, "High Executor Mavren"),
(@MAVREN,     2, 2, "We've been over this before. Only a focused assault upon Deatholme will meet with success. These other targets are distractions that we can ill afford. I suggest that you inform your Captain Helios that he is to leave off his campaign against the Shadowpine trolls and focus solely on the Scourge.", 12, 1, 100, 0, 0, 0, 12569, 0, "High Executor Mavren"),
(@MAVREN,     2, 3, "Perhaps we should let the matter rest for the time being until your chef has prepared your meal? You look a bit piqued if you don't mind my saying. We can resume our discussion when you are feeling more yourself.", 12, 1, 100, 0, 0, 0, 12570, 0, "High Executor Mavren"),
(@VALWYN,     0, 0, "%s whispers something in the Dame's ear.", 16, 0, 100, 0, 0, 0, 13361, 0, "Advisor Valwyn"),
(@VALWYN,     1, 0, "%s looks appalled at the Executor's tone!", 16, 0, 100, 0, 0, 0, 12574, 0, "Advisor Valwyn"),
(@VALWYN,     1, 1, "%s tries to busy herself with other matters.", 16, 0, 100, 0, 0, 0, 12575, 0, "Advisor Valwyn"),
(@VALWYN,     1, 2, "%s glares daggers in the direction of High Executor Mavren and Deathstalker Maltendis.", 16, 0, 100, 0, 0, 0, 12576, 0, "Advisor Valwyn"),
(@VALWYN,     1, 3, "%s lifts her nose and sniffs in response to the High Executor's reply.", 16, 0, 100, 0, 0, 0, 12577, 0, "Advisor Valwyn"),
(@VALWYN,     1, 4, "%s shakes her head in disbelief at what she is hearing out of the Forsaken's mouth.", 16, 0, 100, 0, 0, 0, 12578, 0, "Advisor Valwyn"),
(@VALWYN,     1, 5, "%s blanches as much of the blood drains from her face.", 16, 0, 100, 0, 0, 0, 12579, 0, "Advisor Valwyn"),
(@VALWYN,     1, 6, "%s reddens furiously at what she is hearing in response to her lady's question.", 16, 0, 100, 0, 0, 0, 12580, 0, "Advisor Valwyn"),
(@VALWYN,     1, 7, "%s concentrates on the wall opposite her, clearly ignoring the looks of the Executor and his assistant.", 16, 0, 100, 0, 0, 0, 12581, 0, "Advisor Valwyn"),
(@VALWYN,     2, 0, "%s looks disgusted at the deathstalker's flirtations.", 16, 0, 100, 274, 0, 0, 13367, 0, "Advisor Valwyn"),
(@MALTENDIS,  0, 0, "%s nods in agreement.", 16, 0, 100, 0, 0, 0, 12582, 0, "Deathstalker Maltendis"),
(@MALTENDIS,  0, 1, "%s opens his mouth as if to add something, and then apparently thinks the better of it.", 16, 0, 100, 0, 0, 0, 12583, 0, "Deathstalker Maltendis"),
(@MALTENDIS,  0, 2, "%s looks over at Advisor Valwyn with a smirk on his face.", 16, 0, 100, 0, 0, 0, 12584, 0, "Deathstalker Maltendis"),
(@MALTENDIS,  0, 3, "%s grins at the High Executor's words.", 16, 0, 100, 0, 0, 0, 12585, 0, "Deathstalker Maltendis"),
(@MALTENDIS,  0, 4, "%s appears bored with the whole discussion.", 16, 0, 100, 0, 0, 0, 12586, 0, "Deathstalker Maltendis"),
(@MALTENDIS,  0, 5, "%s winks slyly at Advisor Valwyn.", 16, 0, 100, 0, 0, 0, 12587, 0, "Deathstalker Maltendis"),
(@MALTENDIS,  0, 6, "%s fidgets with his armor, clearly uncomfortable at the tone of the discussion.", 16, 0, 100, 0, 0, 0, 12588, 0, "Deathstalker Maltendis"),
(@MALTENDIS,  0, 7, "%s looks like he'd rather be anywhere else but here.", 16, 0, 100, 0, 0, 0, 12589, 0, "Deathstalker Maltendis"),
(@MALTENDIS,  1, 0, "%s turns to Advisor Valwyn and winks lasciviously in her direction.", 16, 0, 100, 2, 0, 0, 13366, 0, "Deathstalker Maltendis");
--
-- END UPDATING QUERIES
--
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;
