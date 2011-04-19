
DROP TABLE IF EXISTS `quest_comments`;
CREATE TABLE `quest_comments` (`entry` int unsigned NOT NULL, `commentId` int unsigned NOT NULL);

DROP TABLE IF EXISTS `comments`;
CREATE TABLE `comments` (`id` int unsigned NOT NULL, `user` varchar(50) NOT NULL, `body` varchar(100), `rating` int unsigned NOT NULL, `date` varchar(16) NOT NULL, `indent` int unsigned NOT NULL, PRIMARY KEY (`id`));
