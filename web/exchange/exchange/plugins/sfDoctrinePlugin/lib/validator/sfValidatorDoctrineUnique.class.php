<?php

/*
 * This file is part of the symfony package.
 * (c) Fabien Potencier <fabien.potencier@symfony-project.com>
 * 
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

/**
 * sfValidatorDoctrineUnique validates that the uniqueness of a column.
 *
 * Warning: sfValidatorDoctrineUnique is susceptible to race conditions.
 * To avoid this issue, wrap the validation process and the model saving
 * inside a transaction.
 *
 * @package    symfony
 * @subpackage validator
 * @author     Fabien Potencier <fabien.potencier@symfony-project.com>
 * @version    SVN: $Id: sfValidatorDoctrineUnique.class.php 8807 2008-05-06 14:12:28Z fabien $
 */
class sfValidatorDoctrineUnique extends sfValidatorSchema
{
  /**
   * Constructor.
   *
   * @param array  An array of options
   * @param array  An array of error messages
   *
   * @see sfValidatorSchema
   */
  public function __construct($options = array(), $messages = array())
  {
    parent::__construct(null, $options, $messages);
  }

  /**
   * Configures the current validator.
   *
   * Available options:
   *
   *  * model:       The model class (required)
   *  * column:      The unique column name in Doctrine field name format (required)
   *                 If the uniquess is for several columns, you can pass an array of field names
   *  * primary_key: The primary key column name in Doctrine field name format (optional, will be introspected if not provided)
   *                 You can also pass an array if the table has several primary keys
   *  * connection:  The Doctrine connection to use (null by default)
   *
   * @see sfValidatorBase
   */
  protected function configure($options = array(), $messages = array())
  {
    $this->addRequiredOption('model');
    $this->addRequiredOption('column');
    $this->addOption('primary_key', null);
    $this->addOption('connection', null);

    $this->setMessage('invalid', 'An object with the same "%column%" already exist.');
  }

  /**
   * @see sfValidatorBase
   */
  protected function doClean($values)
  {
    $table = Doctrine::getTable($this->getOption('model'));
    if (!is_array($this->getOption('column')))
    {
      $this->setOption('column', array($this->getOption('column')));
    }

    $q = Doctrine_Query::create()
          ->from($this->getOption('model') . ' a');

    foreach ($this->getOption('column') as $column)
    {
      $colName = $table->getColumnName($column);

      $q->addWhere('a.' . $colName . ' = ?', $values[$column]);
    }

    $object = $q->fetchOne();

    // if no object or if we're updating the object, it's ok
    if (!$object || $this->isUpdate($object, $values))
    {
      return $values;
    }

    $error = new sfValidatorError($this, 'invalid', array('column' => implode(', ', $this->getOption('column'))));
    throw new sfValidatorErrorSchema($this, array(implode(', ', $this->getOption('column')) => $error));
  }

  /**
   * Returns whether the object is being updated.
   *
   * @param BaseObject  A Doctrine object
   * @param array       An array of values
   *
   * @param Boolean     true if the object is being updated, false otherwise
   */
  protected function isUpdate(Doctrine_Record $object, $values)
  {
    // check each primary key column
    foreach ($this->getPrimaryKeys() as $column)
    {
      if ($object->$column != $values[$column])
      {
        return false;
      }
    }

    return true;
  }

  /**
   * Returns the primary keys for the model.
   *
   * @return array An array of primary keys
   */
  protected function getPrimaryKeys()
  {
    if (is_null($this->getOption('primary_key')))
    {
      $primaryKeys = Doctrine::getTable($this->getOption('model'))->getIdentifier();
      $this->setOption('primary_key', $primaryKeys);
    }

    if (!is_array($this->getOption('primary_key')))
    {
      $this->setOption('primary_key', array($this->getOption('primary_key')));
    }

    return $this->getOption('primary_key');
  }
}