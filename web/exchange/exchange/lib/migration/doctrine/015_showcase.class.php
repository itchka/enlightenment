<?php
/**
 * This class has been auto-generated by the Doctrine ORM Framework
 */
class Showcase extends Doctrine_Migration
{
	public function up()
	{
		$this->addColumn('theme', 'showcase', 'boolean');
		$this->addColumn('application', 'showcase', 'boolean');
		$this->addColumn('madule', 'showcase', 'boolean');
	}

	public function down()
	{
		$this->removeColumn('madule', 'showcase');
		$this->removeColumn('application', 'showcase');
		$this->removeColumn('theme', 'showcase');
	}
}