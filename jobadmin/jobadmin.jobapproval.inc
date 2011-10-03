<?php

// Build the job approval list/form
function _job_approval_list() {
	$build['admin_joblist_form'] = drupal_get_form('admin_joblist_form');
	return  $build;
}

// Builds the form, unformatted (not a table!)
function admin_joblist_form($form) {

	$header = joblist_headers();

	$query = db_select('jobposting', 'j')->extend('PagerDefault')->extend('TableSort');
	$query->join('node', 'n', 'j.nid = n.nid');
	$query->fields('n', array('nid', 'title', 'created', 'changed'))
	->fields('j', array('expiredate', 'job_type', 'company', 'address_country', 'nid'));

	// Hardcoded to a 50-limit
	$nids = $query
	->limit(50)
	->condition('n.type', 'jobposting', '=')
	->condition('n.status', 0, "=")
	->condition('j.expiredate', REQUEST_TIME, ">" )
	->orderByHeader($header)
	->execute();

	$form['#tree'] = 1;
	$form['node'] = array();
	foreach ($nids as $node) {
		switch($node->job_type) {
			case 0:
				$node->job_type = "";
				break;
			case 1:
				$node->job_type = "Full Time";
				break;
			case 2:
				$node->job_type = "Part Time";
				break;
			case 3:
				$node->job_type = "Contract";
				break;
			case 4:
				$node->job_type = "Internship";
				break;
		}
		$form['node'][$node->nid] = array(
  			'#type' => 'radios', 
  			'#default_value' => 2,
  			'#options' => array( 0,0,0,0),
		);
		$form['node'][$node->nid]['created'] 	= array( '#markup' => format_date($node->created, 'short'));
		$form['node'][$node->nid]['type'] 		= array( '#markup' => $node->job_type);
		$form['node'][$node->nid]['title'] 		= array(
        'data' => array(
          '#type' => 'link',
          '#title' => $node->title,
          '#href' => 'node/' . $node->nid,
          '#suffix' => ' ' . theme('mark', array('type' => node_mark($node->nid, $node->changed))),
		),
		);
		$form['node'][$node->nid]['company'] 	= array( '#markup' => $node->company);
		$form['node'][$node->nid]['country'] 	= array( '#markup' => $node->address_country);
		$form['node'][$node->nid]['expiredate'] = array( '#markup' => date("m/d/y", $node->expiredate));
	}

	$form['actions'] = array('#type' => 'actions');
	$form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Submit'));
	$form['#submit'][] = 'jobadmin_joblist_form_submit';

	return $form;
}

// Theme's the joblist form into a table:
function theme_admin_joblist_form($form) {
	$form = $form['form']; //this can probably be done better in theme_hook

	$rows = array();
	if(isset($form['node'])){
		foreach (element_children($form['node']) as $key) {
			if (isset($form['node'][$key])) {
				$row = array();
				$row[] = drupal_render($form['node'][$key]['created']);
				$row[] = drupal_render($form['node'][$key]['type']);
				$row[] = drupal_render($form['node'][$key]['title']);
				$row[] = drupal_render($form['node'][$key]['company']);
				$row[] = drupal_render($form['node'][$key]['country']);
				$row[] = drupal_render($form['node'][$key]['expiredate']);
				$row[] = array('data' => drupal_render($form['node'][$key][0]));
				$row[] = array('data' => drupal_render($form['node'][$key][1]));
				$row[] = array('data' => drupal_render($form['node'][$key][2]));
				$row[] = array('data' => drupal_render($form['node'][$key][3]));

				$rows[] = $row;
			}
		}
	}
	
	// Individual table headers.
	$output = "";
	$output .= theme('table', array('header' => joblist_headers(), 'rows' => $rows, 'empty' => "No jobs to approve."));
	$output .= theme('pager');
	$output .= drupal_render_children($form);
	return $output;
}

// Return the headers for the job list/form
function joblist_headers() {
	return  array(
	  	'created' 		=> array('data' => t('Date'), 'field' => 'n.created', 'sort' => 'desc'),
	  	'type' 			=> array('data' => t('Type'), 'field' => 'j.job_type'),
	    'title' 		=> array('data' => t('Title'), 'field' => 'n.title'),
	  	'company' 		=> array('data' => t('Company'), 'field' => 'j.company'),
	    'country' 		=> array('data' => t('Country'), 'field' => 'j.address_country'),
	  	'expiredate' 	=> array('data' => t('Expire Date'), 'field' => 'j.expiredate'),
		'approve' 		=> array('data' => t('Approve')),
		'reject' 		=> array('data' => t('Reject')),
		'later' 		=> array('data' => t('Later')),
		'preapprove'    => array('data' => t('PreApprove Jobs'))
	);
}

function jobadmin_preapproved(){

	$output = "";
	$output.= drupal_render(drupal_get_form('jobadmin_preapproved_form'));

	return $output;
}

function jobadmin_joblist_form_submit($form, &$form_state) {

	$preapprove_uids = array(); // list of new preapproved uids

	foreach (element_children($form_state['values']['node']) as $nid) {
		if($form_state['values']['node'][$nid]==2) { //later
			continue;
		} else if($form_state['values']['node'][$nid]== 1) { // delete
			node_delete($nid);
		} else if($form_state['values']['node'][$nid]== 0) { // approve
			$node = node_load($nid);
			$node->status = 1;
			node_save($node);
		}
		else if($form_state['values']['node'][$nid] == 3){ // preapprove
			$node = node_load($nid);
			$node->status = 1;
			node_save($node);
			$preapprove_uids[] = $node->uid; // add to newly preapproved-uids

			$query = db_select('preapproved', 'u');
			$query->fields('u', array('uid'));
			$query ->condition('u.uid', $node->uid , '=');
			$count_query = $query->countQuery()->execute()->fetchField();

			if($count_query !=1){
				$var = db_insert('preapproved');
				$var ->fields(array('uid' =>$node->uid))
				->execute();
			}

			drupal_set_message("Preapproval Success");
		}
	}

	// If we've added any preapprovals, we must
	//  go over all job postings and approve them too.
	$uids = array_unique($preapprove_uids);

	if(!empty($uids)) {

		foreach($uids as $uid) {
				
			$query = db_select('node', 'n');
			$query->fields('n', array('nid'));
			$query->condition('n.type', 'jobposting' , '=');
			$query->condition('n.uid', $uid , '=');
			$query->condition('n.status', 0 , '=');
			$nids = $query->execute();

			foreach ($nids as $nid){
				$node = node_load($nid->nid);
				$node->status = 1;
				node_save($node);
			}
				
		}
	}
}

// Preapproved table headers
function jobadmin_preappovedtable_header(){
	return array(
    'name'   => array('data' => t('Company Name'), 'field' => 'u.name', 'sort' =>'desc'),
    'status'   => array('data' => t('Status'), 'field' => 'u.status'),
    'member_for' => array('data' => t('Member for'), 'field' => 'u.created', 'sort' => 'desc'),
    'access' => array('data' => t('Last access'), 'field' => 'u.access'),
	);
}

// Return the form for preapproved users
function jobadmin_preapproved_form($form){

	$header = jobadmin_preappovedtable_header();

	$query = db_select('users', 'u');
	$query ->join('preapproved', 'v', 'u.uid = v.uid');
	$query->condition('u.uid', 0, '<>');

	$count_query = clone $query;
	$count_query->addExpression('COUNT(u.uid)');

	$query = $query->extend('PagerDefault')->extend('TableSort');
	$query
	->fields('u', array('uid', 'name', 'status', 'created', 'access'))
	->limit(50)
	->orderByHeader($header)
	->setCountQuery($count_query);
	$result = $query->execute();

	$status = array(t('blocked'), t('active'));
	$options = array();
	foreach ($result as $account) {
		$options[$account->uid] = array(
      'name' => $account->name,
      'status' =>  $status[$account->status],
      'member_for' => format_interval(REQUEST_TIME - $account->created),
      'access' =>  $account->access ? t('@time ago', array('@time' => format_interval(REQUEST_TIME - $account->access))) : t('never'),
		);

	}
	$form['entries'] = array(
    	    '#type' => 'tableselect',
            '#header' => $header,
            '#options' => $options,
            '#empty' => t('No PreApproved Job Poster.'),   

	);
	$form['actions'] = array('#type' => 'actions');
	$form['actions']['submit'] = array('#type' => 'submit', '#value' =>t('Remove'));
	$form['#submit'][] = 'jobadmin_preapproved_form_submit';

	return $form;
}

function jobadmin_preapproved_form_submit($form, &$form_state){
	$domains = array_filter($form_state['values']['entries']);
	foreach($domains as $deletedomain){
		$domain_deleted = db_delete('preapproved')
		->condition('uid', $deletedomain)
		->execute();
		drupal_set_message("Pre-Approved Disabled");
	}
}
