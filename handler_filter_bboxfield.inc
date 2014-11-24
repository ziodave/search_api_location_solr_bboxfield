<?php

/**
 * @file
 * Contains SearchApiViewsHandlerFilterLocationArea.
 */

/**
 * Provides a Views filter for location area fields.
 */
class SearchApiLocationSolrBboxfieldViewsHandlerFilter extends SearchApiViewsHandlerFilter {

  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['operator']['default'] = 'Intersects';
    $options['widget']['default'] = '';
    foreach (search_api_location_solr_bboxfield_get_geofield_widget_types() as $id => $widget) {
      $options["widget-$id"]['default'] = array();
    }

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['widget'] = array(
      '#type' => 'select',
      '#title' => t('Widget type'),
      '#description' => t('The type of form element you would like to present to the user for this filter.'),
      '#options' => array('' => t('Raw input')),
      '#default_value' => $this->options['widget'],
    );
    foreach (search_api_location_solr_bboxfield_get_geofield_widget_types() as $id => $widget) {
      $form['widget']['#options'][$id] = $widget['label'];
      $form["widget-$id"] = array(
        '#type' => 'fieldset',
        '#title' => t('Widget settings'),
        '#tree' => TRUE,
        '#dependency' => array(
          'edit-options-widget' => array($id)
        ),
      );
      if (module_hook($widget['module'], 'field_widget_settings_form')) {
        $field = $this->getFieldApiField();
        $instance['widget'] = $widget;
        $instance['widget']['type'] = $id;
        $instance['widget']['settings'] = $this->options["widget-$id"];
        $widget_form = module_invoke($widget['module'], 'field_widget_settings_form', $field, $instance);
        if ($widget_form) {
          $form["widget-$id"] += $widget_form;
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function operator_options() {
    return array(
      'Intersects' => t('Intersects'),
      'IsWithin' => t('Is within'),
      'Contains' => t('Contains'),
      'Disjoint' => t('Disjoint to'),
   );
  }

  /**
   * {@inheritdoc}
   */
  public function value_form(&$form, &$form_state) {
    $widget_id = $this->options['widget'];
    if (empty($form_state['exposed']) || !$widget_id) {
      $form['value'] = array(
        '#type' => 'textfield',
        '#title' => t('Value'),
        '#description' => t('Enter the bounds as comma-separated values, in the format "left,bottom,right,top".'),
        '#size' => 30,
        '#default_value' => isset($this->value) ? $this->value : '',
      );
    }
    else {
      $widget = field_info_widget_types($widget_id);
      if (!$widget) {
        $args = array(
          '%filter' => $this->real_field,
          '%view' => $this->view->name,
          '%widget_type' => $widget_id,
        );
        $this->query->abort(t('Error while displaying filter %filter on view %view: could not find widget type %widget_type.', $args));
        return;
      }
      if (!module_hook($widget['module'], 'field_widget_form')) {
        $args = array(
          '%filter' => $this->real_field,
          '%view' => $this->view->name,
          '%widget_type' => $widget_id,
        );
        $this->query->abort(t('Error while displaying filter %filter on view %view: could not retrieve form for widget type %widget_type.', $args));
        return;
      }
      $field = $this->getFieldApiField();
      $instance['widget'] = $widget;
      $instance['widget']['type'] = $widget_id;
      $instance['widget']['settings'] = $this->options["widget-$widget_id"];
      $langcode = $GLOBALS['language']->language;
      $items = array($this->value ? $this->value : array());
      $delta = 0;
      $parents = isset($form['#parents']) ? $form['#parents'] : array();
      $parents[] = 'value';
      $element = array(
        '#entity' => NULL,
        '#entity_type' => NULL,
        '#bundle' => NULL,
        '#field_name' => $field ? $field['field_name'] : NULL,
        '#language' => $langcode,
        '#field_parents' => $parents,
        '#columns' => $field ? array_keys($field['columns']) : array(),
        '#title' => '',
        '#description' => '',
        '#required' => FALSE,
        '#delta' => $delta,
      );
      $function = $widget['module'] . '_field_widget_form';
      $form['value'] = $function($form, $form_state, $field, $instance, $langcode, $items, $delta, $element);
      $form['value']['#tree'] = TRUE;
    }
  }

  function accept_exposed_input($input) {
    return parent::accept_exposed_input($input);
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    while (isset($this->value[0])) {
      $this->value = $this->value[0];
    }
    if (!empty($this->value['wkt'])) {
      if (empty($this->value['left'])) {
        geofield_compute_values($this->value);
      }
      $bbox = array_intersect_key($this->value, array('left' => 1, 'bottom' => 1, 'right' => 1, 'top' => 1));
    }
    elseif (is_scalar($this->value)) {
      $bbox = SearchApiViewsHandlerArgumentLocationBbox::parseCorners($this->value);
    }
    if (empty($bbox)) {
      return;
    }
    $location_options = $this->query->getOption('search_api_location_solr_bboxfield', array());
    $location_options[] = array(
      'field' => $this->real_field,
      'operator' => $this->operator,
      'bbox' => $bbox,
    );
    $this->query->setOption('search_api_location_solr_bboxfield', $location_options);
  }

  /**
   * Returns the Field API field related to this filter, if any.
   *
   * @return array
   *   An array with the field information of the related field; or an empty
   *   array if none could be found.
   */
  protected function getFieldApiField() {
    foreach (array_reverse(explode(':', $this->real_field)) as $part) {
      if ($field = field_info_field($part)) {
        // @todo Check whether this is a field of a supported type (based on the
        //   currently selected widget)?
        return $field;
      }
    }
    return array();
  }

}