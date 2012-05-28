
DROP TABLE IF EXISTS `comments`;
CREATE TABLE `comments` (`id` int NOT NULL, `user` varchar(50) NOT NULL, `body` varchar(100) NOT NULL, `rating` int NOT NULL, `date` varchar(16) NOT NULL, `indent` int NOT NULL, PRIMARY KEY (`id`));

DROP TABLE IF EXISTS `quest_comments`;
CREATE TABLE `quest_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `achievement_comments`;
CREATE TABLE `achievement_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `npc_comments`;
CREATE TABLE `npc_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `object_comments`;
CREATE TABLE `object_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `faction_comments`;
CREATE TABLE `faction_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `spell_comments`;
CREATE TABLE `spell_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `zone_comments`;
CREATE TABLE `zone_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `itemset_comments`;
CREATE TABLE `itemset_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `item_comments`;
CREATE TABLE `item_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `title_comments`;
CREATE TABLE `title_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));

DROP TABLE IF EXISTS `skill_comments`;
CREATE TABLE `skill_comments` (`entry` int NOT NULL, `commentId` int NOT NULL, PRIMARY KEY (`entry`, `commentId`));
