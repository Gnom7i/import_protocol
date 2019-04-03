<?php
/**
 * Simple page content.
 */
 
 
function athlete_simple_page_content($athlete_id) {	
	{// Переопределяю тип переменной и проверяю на существование
		settype($athlete_id, "integer");
		db_query("SELECT * FROM rating_dvfo_athlete WHERE athlete_id = '{$athlete_id}'")->fetchField() or drupal_not_found();
	}
	// Выборка данных атлета
	$QueryName = db_query("SELECT c.athlete_id, c.name, c.age, c.fo, c.subject_rf, c.city,
								IF(c.gender = 1, 'Женщина', 'Мужчина') AS gender, cntr.name AS country
							FROM rating_dvfo_athlete AS c
							INNER JOIN country AS cntr ON c.country = cntr.iso
							WHERE c.athlete_id = '{$athlete_id}'");
	// Выборка данных для таблицы результатов
	$query = db_query("SELECT IF(c.discipline = 'powerlifting', 
						'по троеборью', 'по жиму лежа') AS discipline, c.total, c.wilks, c.squat, ipf_points,
						c.brench, c.deadlift, c.position, c.date, c.weight, (c.date - a.age) AS age,
						nc.name_ru AS competition, IF(c.devizion = 1, 'БЭ', 'ЭК') AS devizion, 
						ac.reduction AS age_category, ctgr.cat AS category
						FROM competition AS c
						INNER JOIN name_competition AS nc ON c.competition = nc.id
						INNER JOIN age_category AS ac ON c.age_category = ac.id
						INNER JOIN rating_dvfo_athlete AS a USING (athlete_id)
						INNER JOIN category AS ctgr USING (gender)
						WHERE c.athlete_id = {$athlete_id} 
							AND c.weight BETWEEN ctgr.min AND ctgr.max						
						GROUP BY c.id
						ORDER BY c.date DESC, c.tstamp DESC
						");
	// заголовки для таблицы результатов
	$header = array('Год', 'М', 'Соревнование', 'Дев.', 'Воз.кат', 'Кат.', 'Вес', 'П', 'Ж', 'Т', 'С', 
									'IPF points', 'Wilks');
	$rows1 = array();
	$rows2 = array();
	
	
	{// Формирую таблицу лучших результатов
	// Выборка из БД
	function max_result_sql($id, $devizion){
	    $result = db_query("SELECT
	                            IF ({$devizion} = 1, 'БЭ', 'ЭК'),
	                            MAX(squat), 
	                            MAX(brench), 
	                            MAX(deadlift),
	                            MAX(total),
	                            MAX(ipf_points) AS ipf_points,
	                            MAX(wilks) AS wilks
				        FROM competition
				        WHERE athlete_id = {$id} 
				            AND devizion = {$devizion}
				            AND discipline = 'powerlifting'
				            ")->fetchAssoc();
		if ('' == $result['wilks']) return;		            
		return $result;
	}
	function max_result_devizion($id){
	    $eq = max_result_sql($id , 2);
	    $raw = max_result_sql($id , 1);
	    $rows = array($eq, $raw);
	    return $rows;
	}
	$table_max_result = max_result_devizion($athlete_id);	
	// HTML tabe
	$TableMax = theme('table', array(	
				'header' => array('',
													'Присед', 
													'Жим', 
													'Тяга', 
													'Сумма',
													'IPF points', 
													'Вилкс',
													),
				'rows' => $table_max_result,
				'caption' => 'Лучшие результаты',
				));
	} // Конец формирования
	
	
	// Формирование строк таблицы "Спортсмен"
   	while ($ValueName = $QueryName->fetchAssoc()) {	
         // выборка позиции спортсмена в рейтингах
         function position_in_top($devizion, $id, $top, $points){
         		// создаю структуру таблицы под лучшиt результаты всех спортсменов
            db_query("CREATE TABLE IF NOT EXISTS a1 (
                                                        id int not null AUTO_INCREMENT, 
                                                        PRIMARY KEY(id), athlete_id int, 
                                                        points text,                                                        
                                                        fo text, 
                                                        subject_rf text,
                                                        country text
                                                    )");
            $athlete = array();
            $athlete = db_query("SELECT * FROM rating_dvfo_athlete WHERE athlete_id = '{$id}'")->fetchAssoc();
            switch($top){
                case 'страна':
                    $coll = 'country';
                    $content = $athlete['country'];    
                    break;
                case 'фо':
                    $coll = 'fo';
                    $content = $athlete['fo'];
                    break;
                case 'субъект':
                    $coll = 'subject_rf';
                    $content = $athlete['subject_rf'];
                    break;
                case 'город':
                    $coll = 'city';
                    $content = $athlete['city'];
                    break;
                default:
                    $coll = '1';
                    $content = '1';   
            }
            // заполняю таблицу лучших результатов
            $position_in_top = db_query("INSERT a1 SELECT 
                                                     NULL, c1.athlete_id,
                                                     MAX({$points}) AS points,                                       
                                                     fo, 
                                                     subject_rf,
                                                     country 
                                                FROM competition AS c1 
                                                INNER JOIN 
                                                    rating_dvfo_athlete AS a1 ON c1.athlete_id = a1.athlete_id 
                                                WHERE {$coll} = '{$content}'
                                                AND {$coll} IS NOT NULL 
                                                AND {$coll} != '' 
                                                AND gender = {$athlete['gender']}
                                                AND devizion = {$devizion}
                                                AND discipline = 'powerlifting'
                                                GROUP BY c1.athlete_id 
                                                ORDER BY points DESC");
             // id равен позиции в рейтинге                                   
            $position_in_top = db_query("SELECT id FROM a1 WHERE athlete_id = {$id}")->fetchField();	
            db_query("DROP TABLE `a1`");
            if($top == 'страна') {
                $gender = ($athlete['gender'] == 1 ? 'women' : 'men');
                if ('' == $position_in_top) return;
                return l($position_in_top, 
                         "/RUS/rating/{$gender}/O", 
                         array('fragment' => $id . '-' . $devizion));
            }
            return $position_in_top;
        }
        // функция вывода позиции в рейтинге
        function out_top($id, $top){
            $raw =  position_in_top(1, $id, $top, 'ipf_points') . ' &#8656; ' . position_in_top(1, $id, $top, 'wilks');
            $raw = ($raw == '' ? '' : '<br />БЭ: № ' . $raw);
            $eq = position_in_top(2, $id, $top, 'ipf_points') . ' &#8656; ' . position_in_top(2, $id, $top, 'wilks');
            $eq = ($eq == '' ? '' : 'ЭК: № ' . $eq);
            return $eq . $raw;
        }
		// Изменение заголовка страницы
		drupal_set_title("{$ValueName['name']}. Таблица результатов", CHECK_PLAIN);
		$rows1[] = array('ID:', $ValueName['athlete_id'], '',);
		$rows1[] = array('Пол:', $ValueName['gender'], out_top($ValueName['athlete_id'], NULL),);
		$rows1[] = array('Год рождения:', $ValueName['age'], '',);
		$rows1[] = array('Страна', $ValueName['country'], out_top($ValueName['athlete_id'], 'страна'));
		$rows1[] = array('Федеральный округ:', $ValueName['fo'], out_top($ValueName['athlete_id'], 'фо'));
		$rows1[] = array('Регион:', $ValueName['subject_rf'], out_top($ValueName['athlete_id'], 'субъект'));
		$rows1[] = array('Город:', $ValueName['city'], out_top($ValueName['athlete_id'], 'город'));	
	} 
	
	
	// Формирование ячеек таблицы "Результатов"
	$chartWilks = array();
	while ($value = $query->fetchAssoc()) {
		$rows2[] = array(
		  $value['date'],
		  $value['position'],
		  $value['competition'] . ' ' . $value['discipline'], 
		  $value['devizion'],
		  $value['age'] . '/' . $value['age_category'],
		  $value['category'],
		  $value['weight'],
		  $value['squat'],
		  $value['brench'],
		  $value['deadlift'],
		  $value['total'],
		  $value['ipf_points'],
		  $value['wilks'],		   
		); 
	} 
	
  
	$TableAthlete = theme('table', array(
                									'header' => array('Раздел', 
																										'Значение', 
																										array('data' => 'Позиция в рейтинге троеборья (IPF points &#8656; Wilks)'),),				
				'rows' => $rows1,
				'caption' => 'Информация о спортсмене',
				));
  
	$TableCompetition = theme('table', array(
		'header' => $header,
		'rows' => $rows2,
		'caption' => 'Таблица результатов',
		)
	);
  
	

	
	
	{//блок рекламы
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
                $AdSenceAdaptiveContent = '
                    <script async src="//pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>
                    <ins class="adsbygoogle"
                         style="display:block"
                         data-ad-format="autorelaxed"
                         data-ad-client="ca-pub-3589435131109855"
                         data-ad-slot="1571727156"></ins>
                    <script>
                         (adsbygoogle = window.adsbygoogle || []).push({});
                    </script>';
	}
	
	
	
	return "
		<table  width='100%'>
			<tr>
				<td colspan='2'>
                    <center>{$AdSenseAdaptive}</center>
                </td>
			</tr>
			<tr>
				<td style='width: 50%; vertical-align: top'>{$TableAthlete}</td>
				<td style='width: 50%; vertical-align: top'>{$TableMax}</td>
			</tr>
			<tr>
				<td colspan='2'>
                    <center>{$AdSenseAdaptive}</center>
                </td>
			</tr>
			<tr><td colspan='2'>{$TableCompetition}</td></tr>
		</table>";
}            