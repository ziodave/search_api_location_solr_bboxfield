<?php

/**
 * @file
 * Provide views data for Search API location
 */

/**
 * Implements hook_views_data_alter().
 */
function search_api_location_solr_bboxfield_views_data_alter(&$data) {
  foreach (search_api_index_load_multiple(FALSE) as $id => $index) {
    $table = &$data['search_api_index_' . $id];
    $fields = array();
    try {
      $server = $index->server();
      if (!$server) {
        continue;
      }
      // Solr servers will only work if they are version 4.10 or higher. For any
      // other service class, we are content if it supports the
      // "search_api_location_solr_bboxfield" data type.
      if ($server->class == 'search_api_solr_service') {
        /** @var SearchApiServer|SearchApiSolrService $server */
        if (version_compare($server->getSolrConnection()->getSystemInfo()->lucene->{'solr-spec-version'}, '4.10', '<')) {
          continue;
        }
      }
      elseif (!$server->supportsFeature('search_api_data_type_search_api_location_solr_bboxfield')) {
        continue;
      }
      foreach ($index->getFields() as $key => $field) {
        if (!empty($field['real_type']) && search_api_extract_inner_type($field['real_type']) == 'search_api_location_solr_bboxfield') {
          $fields[$key] = $field;
        }
      }
    }
    catch (SearchApiException $e) {
      // Ignore missing servers.
    }
    foreach ($fields as $key => $field) {
      $key = _entity_views_field_identifier($key, $table);
      if (isset($table[$key])) {
        $table[$key]['filter']['handler'] = 'SearchApiLocationSolrBboxfieldViewsHandlerFilter';
        $table[$key]['filter']['type'] = $field['real_type'];
      }
    }
  }
}
