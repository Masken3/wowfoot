
DROP TABLE IF EXISTS `quest_comments`;
CREATE TABLE `quest_comments` (`entry` int NOT NULL, `commentId` int NOT NULL);

DROP TABLE IF EXISTS `comments`;
CREATE TABLE `comments` (`id` int NOT NULL, `user` varchar(50) NOT NULL, `body` varchar(100), `rating` int NOT NULL, `date` varchar(16) NOT NULL, `indent` int NOT NULL, PRIMARY KEY (`id`));
