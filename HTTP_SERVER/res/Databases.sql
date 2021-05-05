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


-- Dump della struttura del database fornitori
CREATE DATABASE IF NOT EXISTS `fornitori` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;
USE `fornitori`;

-- Dump della struttura di tabella fornitori.categorie
CREATE TABLE IF NOT EXISTS `categorie` (
  `nome` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`nome`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Dump dei dati della tabella fornitori.categorie: ~0 rows (circa)
/*!40000 ALTER TABLE `categorie` DISABLE KEYS */;
/*!40000 ALTER TABLE `categorie` ENABLE KEYS */;

-- Dump della struttura di tabella fornitori.fornitori
CREATE TABLE IF NOT EXISTS `fornitori` (
  `ID_fornitore` int(11) NOT NULL AUTO_INCREMENT,
  `nome` varchar(256) NOT NULL,
  `email` varchar(256) DEFAULT NULL,
  `telefono` varchar(256) DEFAULT NULL,
  `paese_provenienza` int(11) NOT NULL,
  PRIMARY KEY (`ID_fornitore`),
  KEY `paese_provenienza` (`paese_provenienza`),
  CONSTRAINT `fornitori_ibfk_1` FOREIGN KEY (`paese_provenienza`) REFERENCES `luoghi` (`ID_luogo`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Dump dei dati della tabella fornitori.fornitori: ~0 rows (circa)
/*!40000 ALTER TABLE `fornitori` DISABLE KEYS */;
/*!40000 ALTER TABLE `fornitori` ENABLE KEYS */;

-- Dump della struttura di tabella fornitori.luoghi
CREATE TABLE IF NOT EXISTS `luoghi` (
  `ID_luogo` int(11) NOT NULL AUTO_INCREMENT,
  `paese` varchar(256) NOT NULL,
  `regione` varchar(256) NOT NULL,
  `città` varchar(256) NOT NULL,
  `via` varchar(256) NOT NULL,
  `numero` varchar(256) NOT NULL,
  PRIMARY KEY (`ID_luogo`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Dump dei dati della tabella fornitori.luoghi: ~0 rows (circa)
/*!40000 ALTER TABLE `luoghi` DISABLE KEYS */;
/*!40000 ALTER TABLE `luoghi` ENABLE KEYS */;

-- Dump della struttura di tabella fornitori.prodotti
CREATE TABLE IF NOT EXISTS `prodotti` (
  `ID_prodotto` int(11) NOT NULL AUTO_INCREMENT,
  `nome` text NOT NULL,
  `opzioni` blob DEFAULT NULL,
  `categoria` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID_prodotto`),
  KEY `categoria` (`categoria`),
  CONSTRAINT `prodotti_ibfk_1` FOREIGN KEY (`categoria`) REFERENCES `categorie` (`nome`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Dump dei dati della tabella fornitori.prodotti: ~0 rows (circa)
/*!40000 ALTER TABLE `prodotti` DISABLE KEYS */;
/*!40000 ALTER TABLE `prodotti` ENABLE KEYS */;

-- Dump della struttura di tabella fornitori.prodotti_fornitori
CREATE TABLE IF NOT EXISTS `prodotti_fornitori` (
  `ID_fornitore` int(11) NOT NULL,
  `ID_prodotto` int(11) NOT NULL,
  `ID_luogo` int(11) NOT NULL,
  `prezzo` double DEFAULT 0,
  `quantita` int(11) DEFAULT 0,
  PRIMARY KEY (`ID_fornitore`,`ID_prodotto`,`ID_luogo`),
  KEY `ID_luogo` (`ID_luogo`),
  KEY `ID_prodotto` (`ID_prodotto`),
  CONSTRAINT `prodotti_fornitori_ibfk_1` FOREIGN KEY (`ID_luogo`) REFERENCES `luoghi` (`ID_luogo`),
  CONSTRAINT `prodotti_fornitori_ibfk_2` FOREIGN KEY (`ID_prodotto`) REFERENCES `prodotti` (`ID_prodotto`),
  CONSTRAINT `prodotti_fornitori_ibfk_3` FOREIGN KEY (`ID_fornitore`) REFERENCES `fornitori` (`ID_fornitore`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Dump dei dati della tabella fornitori.prodotti_fornitori: ~0 rows (circa)
/*!40000 ALTER TABLE `prodotti_fornitori` DISABLE KEYS */;
/*!40000 ALTER TABLE `prodotti_fornitori` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
