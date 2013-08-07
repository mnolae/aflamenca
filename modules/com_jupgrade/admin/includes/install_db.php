<?php
/**
 * jUpgrade
 *
 * @version		$Id$
 * @package		MatWare
 * @subpackage	com_jupgrade
 * @copyright	Copyright 2006 - 2011 Matias Aguire. All rights reserved.
 * @license		GNU General Public License version 2 or later.
 * @author		Matias Aguirre <maguirre@matware.com.ar>
 * @link		http://www.matware.com.ar
 */

define('_JEXEC', 1);
define('JPATH_BASE', dirname(__FILE__));
define('DS', DIRECTORY_SEPARATOR);

$directory = '';

if (ctype_alpha($_GET['directory'])) {
	$directory = $_GET['directory'];
}

require_once JPATH_BASE.'/defines.php';
if (file_exists(JPATH_LIBRARIES.'/joomla/import.php')) {
	require_once JPATH_LIBRARIES.'/joomla/import.php';
}else if (file_exists(JPATH_LIBRARIES.'/import.php')) {
	require_once JPATH_LIBRARIES.'/import.php';
}
if (file_exists(JPATH_LIBRARIES.'/cms/model/legacy.php')) {
	require_once JPATH_LIBRARIES.'/cms/model/legacy.php';
}
require_once JPATH_LIBRARIES.'/joomla/methods.php';
require_once JPATH_LIBRARIES.'/joomla/factory.php';
require_once JPATH_LIBRARIES.'/joomla/error/error.php';
require_once JPATH_LIBRARIES.'/joomla/base/object.php';
require_once JPATH_LIBRARIES.'/joomla/database/database.php';
require_once JPATH_INSTALLATION.'/models/database.php';

require_once JPATH_LIBRARIES.'/cms/schema/changeset.php';
require_once JPATH_ADMINISTRATOR.'/components/com_installer/models/database.php';

// jUpgrade class
require_once JPATH_BASE.'/jupgrade.class.php';

// jUpgrade class
$jupgrade = new jUpgrade;

// Getting the component parameter with global settings
$params = $jupgrade->getParams();

// getting config
$jconfig = new JConfig();

$config = array();
$config['dbo'] = & JInstallationHelperDatabase::getDBO('mysql', $jconfig->host, $jconfig->user, $jconfig->password, $jconfig->db, $params->prefix_new);

// getting helper
$installHelper = new JInstallationModelDatabase($config);

// installing global database
$schema = JPATH_INSTALLATION.'/sql/mysql/joomla.sql';

if (!$installHelper->populateDatabase($config['dbo'], $schema) > 0) {
	return false;
	exit;
}

$query = "UPDATE #__modules SET `published` = '0' WHERE `id` IN (1,16,17)";
$jupgrade->db_new->setQuery($query);
$jupgrade->db_new->query();

// Check for query error.
$error = $jupgrade->db_new->getErrorMsg();

if ($error) {
	return false;
	exit;
}

// Fixing the database schema
$changeSet = $jupgrade->getChangeSet();
$changeSet->fix();

$jupgrade->fixSchemaVersion($changeSet);
