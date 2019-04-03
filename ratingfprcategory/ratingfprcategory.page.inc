<?php

function ratingfprcategory_pagedefault(){  // Страница по умолчанию
	$video = '<iframe width="560" height="315" src="https://www.youtube.com/embed/mXvIaUTXNfE" frameborder="0" allowfullscreen></iframe>';	
	return "<table>
				<tr>
					<td>
						Для просмотра рейтингов по весовым категорям воспользуйтесь вкладками находящимися над этим текстом. 
						Если Вас нет в рейтнгах, присылайте протокол соревнований через обратную связь.
					</td>					
				</tr>
				<tr>
					<td>{$video}</td>
				</tr>
			</table>
			";
} 


function texture_top($devizion, $gender, $category){
	{//преабразую аргументы URL для запросов
		if($gender == 'woomen') $gender = 1;
		if($gender == 'men') $gender = 2;
		if($devizion == 'raw') $devizion = 1;
		if($devizion == 'eq') $devizion = 2;
	}
	
	
	{//запросы к БД
		//выборка данных о весовой с помощью аргументов URL
		$SqlCat = db_query("SELECT min, max FROM category
							WHERE gender = '{$gender}' AND cat = '{$category}'
						  ")->fetchAssoc() or drupal_not_found();						  		
		//выборка названий категорий
		$SqlCatName = db_query("SELECT cat FROM category
								WHERE gender = '{$gender}'								
		")->fetchCol();


		//создание временной таблицы
		db_query("
			CREATE TEMPORARY TABLE tmp_cat 
			SELECT c.athlete_id, name, MAX(wilks) as w FROM competition AS c
			 INNER JOIN rating_dvfo_athlete AS a ON c.athlete_id = a.athlete_id
			WHERE weight BETWEEN {$SqlCat['min']} AND {$SqlCat['max']}
			 AND country = 643
			 AND devizion = {$devizion}
			 AND gender = {$gender}
			GROUP BY c.athlete_id
			 HAVING MAX(wilks)
			ORDER BY w DESC
			LIMIT 100;
		");		
		//формирую таблицу вывода
		$RowAthlete = 
		db_query("
			SELECT a.athlete_id, a.name, c.wilks, 
				CONCAT(a.age, '/', (c.date - a.age)) AS age, 
				CONCAT(s.alis, '<br />', a.city) region, 
				CONCAT(c.total, '/', c.weight) result, 
				CONCAT(nc.name_ru, ' ', c.date) comp 
			FROM tmp_cat AS tmp
			INNER JOIN rating_dvfo_athlete a USING(athlete_id)
			INNER JOIN competition c ON wilks = w AND c.athlete_id = tmp.athlete_id
			INNER JOIN name_competition nc ON nc.id = c.competition
			INNER JOIN subject s ON a.subject_rf = s.name
			GROUP BY c.athlete_id
			ORDER BY c.wilks DESC;
		");
		//удаляю временную таблицу
		db_query('DROP TABLE tmp_cat');	
	}
	
	
	
	// Формирование таблицы
  $header = array(t('№'), 
                  t('Имя'), 
                  t('Возраст'),
//                  t('Субъект'), 
                  t('Вилкс'),
//                  t('Сумма/Вес'),  
                  t('Соревнования'), 
                  );
  $rows = array();
  $i = 1;
  while ($value = $RowAthlete->fetchAssoc()) {
    $rows[] = array(
      $value['number'] = $i++ . '.',
      array('data' => "<a href='/athlete/{$value['athlete_id']}'>{$value['name']}</a>"),
	  $value['age'],
//	  $value['region'],
	  $value['wilks'],
//	  $value['result'],
	  $value['comp'],
	  
    ); 
  } 
  
  
  return theme('table', array(
                          'header' => $header, 
                          'rows' => $rows, 
                          'attributes' => array('class' => array('MyTable')),
                          'caption' => $caption_table = 
														($devizion == 1 ? 'Классическое троеборье' : 'Троеборье'),
                          'empty' => 'Таблица пуста. Ищем таланты!',
                        )
              );
}


function ratingfprcategory_top($gender, $category){
	$eq = texture_top('eq', $gender, $category);
	$raw = texture_top('raw', $gender, $category);
	global $user;
	if($user->uid != 1) {
    		$AdSenseAdaptive = '<p><center>
    		<script async src="//pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>
    		<!-- HEAD Rating -->
    		<ins class="adsbygoogle"
    			 style="display:block"
    			 data-ad-client="ca-pub-3589435131109855"
    			 data-ad-slot="5272531152"
    			 data-ad-format="auto"></ins>
    		<script>
    		(adsbygoogle = window.adsbygoogle || []).push({});
    		</script></center></p>';
	}
	else {$AdSenseAdaptive = "{Блок рекламы}";}
	$conclusion = "
	<table>
		<tr>
			<td colspan='2'><center>{$AdSenseAdaptive}</center></td>
		</tr>
		<tr>
			<td style='width: 50%; vertical-align: top'>{$eq}</td>
			<td style='width: 50%; vertical-align: top'>{$raw}</td>
		</tr>
	</table>";
	return $conclusion; 	
} 