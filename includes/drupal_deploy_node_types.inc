<?php
  
function drupal_deploy_export_node_types($filename = ''){
  $items = drupal_deploy_export_node_types_fetch_data();
  
  if(empty($filename)){
    $filename = 'node_types.export';
  }
  drupal_deploy_export_save($items, $filename);
}

function drupal_deploy_import_node_types($filename){
  if(empty($filename)){
      $filename = 'node_types.export';
  }
  if(!is_file($filename)){
    drupal_set_message(t('File !filename does not exists', array('!filename' => $filename)), 'error');
    return FALSE;
  }
  $data = file_get_contents($filename);
  if(!$data = json_decode($data, TRUE)){
    drupal_set_message(t('Json decode error!'), 'error');
  }
  drupal_deploy_import_node_types_items($data);
}

function drupal_deploy_export_node_types_fetch_data()
{
  $items = array();
  $result = db_query("SELECT * FROM {node_type} ORDER BY type");
  while ($db_item = db_fetch_object($result)) {
    
    $item = array(
      'type' => $db_item->type,
      'name' => $db_item->name,
      'module' => $db_item->module,
      'has_title' => $db_item->has_title,
      'title_label' => $db_item->title_label,
      'has_body' => $db_item->has_body,
      'body_label' => $db_item->body_label,
      'description' => $db_item->description,
      'help' => $db_item->help,
      'min_word_count' => $db_item->min_word_count,
      'custom' => $db_item->custom,
      'modified' => $db_item->modified,
      'locked' => $db_item->locked,
      'variables' => drupal_deploy_export_get_variables($db_item),
    );
    $items[] = $item;
  }
  return $items;
}

function drupal_deploy_export_get_variables($type){
  $form_id = 'node_type_form';
  $form_state = array();
  $form = node_type_form($form_state, $type);
  drupal_prepare_form($form_id, $form, $form_state);

  $return = array();
  
  unset($form['identity']);
  unset($form['submission']);
  unset($form['old_type']);
  unset($form['orig_type']);
  unset($form['module']);
  unset($form['custom']);
  unset($form['modified']);
  unset($form['locked']);
  unset($form['submit']);
  unset($form['reset']);  
  unset($form['delete']);
  unset($form['form_id']);

  foreach($form as $key => $val){
    if($key[0] == '#')
    {
      continue;
    }else{
      $return[$key] = _process_form_variable($val);
    }
  }
  
  return $return;
}

function _process_form_variable($array)
{
  if(isset($array['#type'])){
    switch($array['#type']){
      case 'fieldset':
      case 'item':
        $return = array();
        foreach($array as $key => $val){
          if($key[0] == '#')
          {
            continue;
          }else{
            $return[$key] = _process_form_variable($val);
          }
        }
        return $return;
        break;
      case 'checkbox':
      case 'textarea':
      case 'textfield':
        return $array['#default_value'];
        break;
      case 'radios':
      case 'select':
        return $array['#options'][$array['#default_value']];
        break;
      case 'checkboxes':
        $return = array();
        foreach($array['#default_value'] as $val){
           $return[] =  $array['#options'][$val];
        }
        return $return;
    }
  }
}