<?php
function jobadmin_list_output() {
	
    $output = "";
	$output .= drupal_render(drupal_get_form('jobadmin_block_form'));
	$header = array(
    'username' => array('data' => t('Username'), 'field' => 'u.name'),
    'status' => array('data' => t('Status'), 'field' => 'u.status'),
    'roles' => array('data' => t('Roles')),
    'member_for' => array('data' => t('Member for'), 'field' => 'u.created', 'sort' => 'desc'),
    'access' => array('data' => t('Last access'), 'field' => 'u.access'),
    'operations' => array('data' => t('Operations')),
  );
  
  
  //parse URL for query keywords
  $queryURL = drupal_get_destination();
  $queryURL = urldecode($queryURL['destination']); //decode to convert any %20's or whatnot
  $split_URL = drupal_parse_url($queryURL);
  $keywords = explode('/', $split_URL['path']);
  
  
  //set up base query
 $query = db_select('users', 'u');
  $query->condition('u.uid', 0, '<>');
  jobadmin_build_filter_query($query);

  $count_query = clone $query;
  $count_query->addExpression('COUNT(u.uid)');
 
  $query = $query->extend('PagerDefault')->extend('TableSort');
  $query
    ->fields('u', array('uid', 'name', 'status', 'created', 'access'))
    ->limit(50)
    ->orderByHeader($header)
    ->setCountQuery($count_query);
  $result = $query->execute();

    //'WHERE (title || email || phone || aboutme) LIKE keyword' if keyword exists
    if(isset($keywords[1])) {
    	$query->condition( 'name', '%'.$keywords[1].'%', 'LIKE');
    }
    
    //'AND (city || address || province || country) LIKE location_keyword' if location input
	if(isset($split_URL['query']['rol'])) {
    	$query->condition(db_or()	->condition( 'address_city', '%'.$split_URL['query']['rol'].'%', 'LIKE' )
    							 	->condition( 'address_street', '%'.$split_URL['query']['rol'].'%', 'LIKE' )
    					 			->condition( 'address_province', '%'.$split_URL['query']['rol'].'%', 'LIKE' )
    								->condition( 'address_country', '%'.$split_URL['query']['rol'].'%', 'LIKE' ));
    }
    
    //'AND jobtype IN selected_jobtypes' if jobtype selected
	if(isset($split_URL['query']['jtyp'])) {
	   $ar = explode(',',$split_URL['query']['jtyp']);
	   if (!in_array('0', $ar)) {
	     $query->condition( 'job_type', $ar , 'IN');
	   }
       
    }
    
    //'AND educationlevel IN selected_edlevels' if edlevel selected
	if(isset($split_URL['query']['ed'])) {
	    $ar = explode(',',$split_URL['query']['ed']);
	    if (!in_array('0', $ar)) {
    	   $query->condition( 'education_level', $ar, 'IN');
	    }
    }

  
  //execute query
  $result = $query->execute();
  $rows = array();
  $rss_items = array();
  foreach ($result as $jobpost) {
  	$rss_items[] = $jobpost->nid;
  	
      //Display Education Level as string, instead of integer representation      
      switch($jobpost->education_level) {
          case 0:
            $jobpost->education_level = "";
            break;
          case 1:
            $jobpost->education_level = "BSc";
            break;
          case 2:
            $jobpost->education_level = "Masters";
            break;
          case 3:
            $jobpost->education_level = "Ph.D";
            break;  
      }
      
      //Display Job Type as string, instead of integer representation     
      switch($jobpost->job_type) {
          case 0:
            $jobpost->job_type = "";
            break;
          case 1:
            $jobpost->job_type = "Full Time";
            break;
          case 2:
            $jobpost->job_type = "Part Time";
            break;
          case 3:
            $jobpost->job_type = "Contract";
            break;
          case 4:
            $jobpost->job_type = "Internship";
            break;    
    }
    
  
    //get nid of job poster profile to link to the results
    $company = db_select('jobposterprofile', 'jpp')
        ->fields('jpp', array('nid'))
        ->condition('uid', $jobpost->uid, '=')
        ->execute()
        ->fetchfield();
        
    //save results
    if ($company){ //if company has a profile
      $rows[] = array(
      array('data' => format_date($jobpost->changed, 'short'), 'class' => array('nowrap')),  
      l($jobpost->title, 'node/' . $jobpost->nid), //link to Job Posting
        l($jobpost->company, 'node/' . $company),  //link to Company Profile
        $jobpost->job_type,
        $jobpost->education_level,
        $jobpost->address_city,
        $jobpost->address_province,
        $jobpost->address_country,
        array('data' => format_date($jobpost->expiredate, 'short'), 'class' => array('nowrap')), 
        );
    }
    else { //no profile (DB Scrape?)
        $rows[] = array(
        array('data' => format_date($jobpost->changed, 'short'), 'class' => array('nowrap')),  
        l($jobpost->title, 'node/' . $jobpost->nid), //link to Job Posting
          $jobpost->company,
          $jobpost->job_type,
          $jobpost->education_level,
          $jobpost->address_city,
          $jobpost->address_province,
          $jobpost->address_country,
          array('data' => format_date($jobpost->expiredate, 'short'), 'class' => array('nowrap')), 
          );
      }
  }
  
  if(isset($split_URL['query']['rss'])) {
  
  return node_feed($rss_items, array());
  	
  }

  //build table
  $build['job_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No available job postings meet your search criteria.'),
  );
  $build['job_pager'] = array('#theme' => 'pager');
  $output .= drupal_render($build);
  
  //$output .= l('RSS', node_feed($rss_items, array()));
  $split_URL['query']['rss'] = TRUE;
  //$output .= l("RSS", url($split_URL['path'], $split_URL));
  $output .= '<div class ="rss_link" target=""><a href="'.url($split_URL['path'], $split_URL).'">[RSS]</a></div>';
  
//  $output .= l
  return $output;
}



/**
 * Implements hook_forms().
 */
function jobadmin_forms() {
	$forms['jobadmin_block_form']= array(
    'callback' => 'jobadmin_box',
    'callback arguments' => array('jobadmin_block_form'),
	);
	return $forms;
}

function jobadmin_block_view($delta = '') {
	// This example comes from node.module. Note that you can also return a
	// renderable array rather than rendered HTML for 'content'.
	$block = array();

	switch ($delta) {
		case 'jobadmin':
			$block['subject'] = t('Start Your Career Here');
			$block['content'] = jobadmin_output();

			break;
	}
	return $block;
}

/**
 * Form builder; Output a search form for the search block's search box.
 *
 * @ingroup forms
 * @see search_box_form_submit()
 * @see search-theme-form.tpl.php
 * @see search-block-form.tpl.php
 */
function jobadmin_box($form, &$form_state, $form_id) {
	$form[$form_id] = array(
    '#type' => 'textfield',
    '#title' => t('Job Search'),
    '#title_display' => 'invisible',
    '#size' => 25,
    '#default_value' => '',
    '#attributes' => array('title' => t('Enter the terms you wish to search for.')),
	);
	$form['advanced'] = array(
    '#title' => t('Advanced Search'),
    '#type' => 'fieldset', 
    '#collapsible' => TRUE, 
    '#collapsed' => TRUE
	);
	$form['advanced']['location'] = array(
    '#type' => 'textfield',
    '#title' => t('Location'),
    '#size' => 25,
    '#description' => t('Enter either a City, Province, State or Country to search for.'),
	);
	$form['education_options'] = array(
    '#type' => 'value', 
    '#value' => array(t('Any'), t('Bachelor\'s degree'), t('Master\'s degree'), t('Doctoral degree')) 
	);
	$form['advanced']['educationlevel'] = array(
    '#title' => t('Education Level'), 
    '#type' => 'select', 
	'#multiple' => TRUE,
	'#size'	=> count($form['education_options']['#value']),
    '#description' => t('Your Highest Credential'),
    '#options' => $form['education_options']['#value'] 
	);
	$form['jobtype_options'] = array(
    '#type' => 'value', 
    '#value' => array(t('Any'), t('Full Time'), t('Part Time'), t('Internship'), t('Contract')) 
	);
	$form['advanced']['jobtype'] = array(
    '#title' => t('Job Type'), 
    '#type' => 'select', 
	'#multiple' => TRUE,
    '#description' => t('Job Type of Interest'),
	'#size'	=> count($form['education_options']['#value']),
    '#options' => $form['jobtype_options']['#value'] 
	);
	$form['actions'] = array('#type' => 'actions');
	$form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Search'));
	$form['#submit'][] = 'jobadmin_box_form_submit';

	return $form;
}


/**
 * Process a block search form submission.
 */
function jobadmin_box_form_submit($form, &$form_state) {
	// The search form relies on control of the redirect destination for its
	// functionality, so we override any static destination set in the request,
	// for example by drupal_access_denied() or drupal_not_found()
	// (see http://drupal.org/node/292565).
	if (isset($_GET['destination'])) {
		unset($_GET['destination']);
	}


	$form_id = $form['form_id']['#value'];
	$info = search_get_default_module_info();
	if ($info) {
		$options = array();
		if(isset($form_state['values']['jobtype']) && !empty($form_state['values']['jobtype'])) {
			$comma_separated = implode(",", $form_state['values']['jobtype']);
			$options['jtyp'] = $comma_separated;
		} 
		if(isset($form_state['values']['educationlevel']) && !empty($form_state['values']['educationlevel'])) {
			$comma_separated = implode(",", $form_state['values']['educationlevel']);
			$options['ed'] = $comma_separated;
		}  
		if(isset($form_state['values']['location']) && !empty($form_state['values']['location'])) {
			$options['rol'] = $form_state['values']['location'];
		} 
		$form_state['redirect'] = array('jobadmin/' . trim($form_state['values'][$form_id]),  array('query'=>$options));
	}
	else {
		form_set_error(NULL, t('Search is currently disabled.'), 'error');
	}
}

