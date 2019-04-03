<?php
function xray_simple_page_content() {
  $nodes = db_query("SELECT * FROM {suroveckij_faq} ");
  $nodes = $nodes->fetchAssoc();

  
  /* $render_array = array(
    // The theme function to apply to the #items.
    '#theme' => 'item_list',
    // The list itself.
    '#items' => $nodes,
    '#title' => t('Argument Information'),
  ); */
  
  
  $build = array(
      '#markup' => $nodes,
      '#prefix' => '<div id="first-time">',
      '#suffix' => '</div>',
      );
      
      
  return $build;
}