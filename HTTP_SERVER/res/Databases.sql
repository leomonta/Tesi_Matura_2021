-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Versione server:              10.4.14-MariaDB - mariadb.org binary distribution
-- S.O. server:                  Win64
-- HeidiSQL Versione:            11.0.0.5919
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Dump della struttura del database mb_db
CREATE DATABASE IF NOT EXISTS `mb_db` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;
USE `mb_db`;

-- Dump della struttura di tabella mb_db.places
CREATE TABLE IF NOT EXISTS `places` (
  `ID_place` int(11) NOT NULL AUTO_INCREMENT,
  `country` varchar(256) NOT NULL,
  `region` varchar(256) NOT NULL,
  `city` varchar(256) NOT NULL,
  `street` varchar(256) NOT NULL,
  `number` varchar(256) NOT NULL,
  PRIMARY KEY (`ID_place`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4;

-- L’esportazione dei dati non era selezionata.

-- Dump della struttura di tabella mb_db.products
CREATE TABLE IF NOT EXISTS `products` (
  `ID_product` int(11) NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `options` blob DEFAULT NULL,
  `tag` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`ID_product`),
  KEY `tag` (`tag`),
  CONSTRAINT `products_ibfk_1` FOREIGN KEY (`tag`) REFERENCES `tags` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4;

-- L’esportazione dei dati non era selezionata.

-- Dump della struttura di tabella mb_db.product_supplier
CREATE TABLE IF NOT EXISTS `product_supplier` (
  `ID_supplier` int(11) NOT NULL,
  `ID_product` int(11) NOT NULL,
  `price` double DEFAULT 0,
  `quantity` int(11) DEFAULT 0,
  PRIMARY KEY (`ID_supplier`,`ID_product`),
  KEY `ID_product` (`ID_product`),
  CONSTRAINT `product_supplier_ibfk_1` FOREIGN KEY (`ID_product`) REFERENCES `products` (`ID_product`),
  CONSTRAINT `product_supplier_ibfk_2` FOREIGN KEY (`ID_supplier`) REFERENCES `suppliers` (`ID_supplier`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- L’esportazione dei dati non era selezionata.

-- Dump della struttura di tabella mb_db.suppliers
CREATE TABLE IF NOT EXISTS `suppliers` (
  `ID_supplier` int(11) NOT NULL AUTO_INCREMENT,
  `company_name` varchar(256) NOT NULL,
  `email` varchar(256) DEFAULT NULL,
  `website` varchar(256) DEFAULT NULL,
  `ID_place` int(11) NOT NULL,
  PRIMARY KEY (`ID_supplier`),
  KEY `ID_place` (`ID_place`),
  CONSTRAINT `suppliers_ibfk_1` FOREIGN KEY (`ID_place`) REFERENCES `places` (`ID_place`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4;

-- L’esportazione dei dati non era selezionata.

-- Dump della struttura di tabella mb_db.tags
CREATE TABLE IF NOT EXISTS `tags` (
  `name` varchar(50) NOT NULL,
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- L’esportazione dei dati non era selezionata.

-- Dump della struttura di tabella mb_db.types
CREATE TABLE IF NOT EXISTS `types` (
  `type` text DEFAULT NULL,
  `content` mediumtext DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- L’esportazione dei dati non era selezionata.

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
