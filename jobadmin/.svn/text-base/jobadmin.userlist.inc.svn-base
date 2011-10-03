<?php

function _user_list(){
	
    $build['_setup_filters_form'] = drupal_get_form('_setup_filters_form');
  	$build['admin_search_form'] = drupal_get_form('admin_search_form');
    return  $build;
}


function _setup_filters_form(){

  $form['Search Filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search Filters'),
    '#attributes' => array('class' => array('container-inline')),
    '#collapsible' => TRUE, 
	'#collapsed' => TRUE,
    '#prefix' => t('<DIV ALIGN=CENTER>'),
  	'#suffix' => t('</DIV>'),
  );

  $form['Search Filters']['administratorFilter'] = array(
   '#type' => 'link', 
  '#title' => t('All'), 
  '#href' => "user/$account->uid/edit", 
  '#options' => array('query' => $destination),
  '#suffix' => t(' | ')
  );

  $form['Search Filters']['jobposterFilter'] = array(
   '#type' => 'link', 
  '#title' => t('JobPoster'), 
  '#href' => "user/$account->uid/edit", 
  '#options' => array('query' => $destination),
  '#suffix' => t(' | ')
  );

  $form['Search Filters']['jobseekerFilter'] = array(
   '#type' => 'link', 
  '#title' => t('JobSeeker'), 
  '#href' => "user/$account->uid/edit", 
  '#options' => array('query' => $destination),
  );


  //Manage Admin Block -------------------------START-------------------
  $form['Manage Admin'] = array(
   '#type' => 'link', 
  '#title' => t('Manage Administrators'), 
  '#href' => "user/$account->uid/edit", 
  '#options' => array('query' => $destination),
  '#prefix' => t('<DIV ALIGN=CENTER>'),
  '#suffix' => t('</DIV> <br></br>'),
  );
  //Manage Admin Block -------------------------END---------------------




  //Search Row Block ---------------------------START---------------------
  $form['search'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search and search options'),
    '#attributes' => array('class' => array('container-inline')),
    '#collapsible' => TRUE, 
	'#collapsed' => FALSE,
  );


  $form['search']['searchBox'] = array(
    '#type' => 'textfield',
    '#title' => t('Search Box'),
  );

  $form['search']['select'] = array(
    '#type' => 'select',
    '#options' => array('username' => t('UID'), 'email' => t('E-mail')),
  );


  $form['search']['submitButton'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );

  //Search Row Block -------------------------------END-----------------------


  return $form;

}

function admin_search_form($form) {
 

  $header = userlist_headers();

  $query = db_select('users', 'u')->extend('PagerDefault')->extend('TableSort');

  $count_query = clone $query;
  $count_query->addExpression('COUNT(u.uid)');

  $query
  ->fields('u', array('uid', 'mail', 'name', 'status', 'created', 'access'))
  ->limit(50)
  ->orderByHeader($header)
  ->setCountQuery($count_query);

  $result = $query->execute();


    $form['account'] = array();
    $users_roles = array();
    
  foreach ($result as $account) {

   
    $roles_result = db_query('SELECT rid FROM {users_roles} WHERE uid = :uid', array(':uid' => $account->uid));
    foreach ($roles_result as $user_role) {
      $users_roles[] = $roles[$user_role->rid];
    }
    
    print $users_roles;
    asort($users_roles);
    
    

    $form['account'][$account->uid]['uid'] 	= array( '#markup' => $account->uid);
    $form['account'][$account->uid]['mail'] 		= array( '#markup' => $account->mail);
    $form['account'][$account->uid]['status'] 		= array( '#markup' => $account->status);

    $form['account'][$account->uid]['role'] 	= array( '#markup' => $users_roles);
    $form['account'][$account->uid]['created'] = array( '#markup' => date("m/d/y", $account->created));
    $form['account'][$account->uid]['access']  = array( '#markup' => date("m/d/y", $account->access));
    
    $form['account'][$account->nid][0] = array(
  			'#type' => 'checkbox', 
  			'#default_value' => "1",
    );
    
   $form['account'][$account->nid][1] = array(
  			'#type' => 'checkbox', 
  			'#default_value' => "1",
    );
    
   $form['account'][$account->nid][2] = array(
  			'#type' => 'checkbox', 
  			'#default_value' => "1",
    );

  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Submit'));
  $form['#submit'][] = 'jobadmin_admin_search_submit';

  return $form;
}

function theme_admin_search_form($form) {
	$form = $form['form']; //this can probably be done better in theme_hook
	
	$rows = array();
	if(isset($form['account'])){
		foreach (element_children($form['account']) as $key) {
			if (isset($form['account'][$key])) {			
				$row = array();
				$row[] = drupal_render($form['account'][$key]['uid']);
				$row[] = drupal_render($form['account'][$key]['mail']);
				$row[] = drupal_render($form['account'][$key]['status']);
				$row[] = drupal_render($form['account'][$key]['role']);
				$row[] = drupal_render($form['account'][$key]['created']);
				$row[] = drupal_render($form['account'][$key]['access']);
	            $row[] = array('data' => drupal_render($form['account'][$key][0]));
	            $row[] = array('data' => drupal_render($form['account'][$key][1]));
	            $row[] = array('data' => drupal_render($form['account'][$key][2]));
	            
				$rows[] = $row;
			}
		}
	}

	// Individual table headers.
	$output = "";
	$output .= theme('table', array('header' => userlist_headers(), 'rows' => $rows, 'empty' => "No Users"));
	$output .= theme('pager');
	$output .= drupal_render_children($form);
	return $output;
}

function userlist_headers() {
	return  array(
    'username' => array('data' => t('UID'), 'field' => 'u.name'),
	'email' => array('data' => t('E-mail'), 'field' => 'u.mail'),
    'status' => array('data' => t('status'), 'field' => 'u.status'),
    'roles' => array('data' => t('Roles')),
    'member_for' => array('data' => t('Member since'), 'field' => 'u.created', 'sort' => 'desc'),
    'access' => array('data' => t('Last access'), 'field' => 'u.access'),
	'disabled' => array('data' => t('Disabled')),  
	'active' => array('data' => t('Active')), 
	'preapproved' => array('data' => t('Pre-Approved')), 
	
  );
}

function jobadmin_admin_search_submit($form, &$form_state) {
  
  drupal_set_message(t('Form Submit'));
    
}