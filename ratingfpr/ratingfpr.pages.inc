<?php
/**
 * Page callback.
 */
 
 
 
// Функция перенаправляющая на вкладку по умолчанию
function ratingfpr_page_default($gender) {
 drupal_goto("/RUS/rating/{$gender}/O");
} 
 
 
// Функция формирующая HTML таблицы для вкладок
function html_table_ratingfpr_page($gender, $devizion, $AgeABBR) {
  // обработка аргументов передаваемых вкладками
  // пол
  if ($gender == 'men') {$gender = 2; $caption = ' мужчин ';}
  if ($gender == 'women') {$gender = 1; $caption = ' женщин ';}
  // возрастные категории
  if ($AgeABBR == 'SJ') {$Age = array('min' => 1, 'max' => 18);  $caption_age = 'Юноши ';}
  if ($AgeABBR == 'J') {$Age = array('min' => 19, 'max' => 23); $caption_age = 'Юниоры ';}
  if ($AgeABBR == 'M1') {$Age = array('min' => 40, 'max' => 49); $caption_age = "Ветераны {$Age['min']} - {$Age['max']} ";}
  if ($AgeABBR == 'M2') {$Age = array('min' => 50, 'max' => 59); $caption_age = "Ветераны {$Age['min']} - {$Age['max']} ";}
  if ($AgeABBR == 'M3') {$Age = array('min' => 60, 'max' => 69); $caption_age = "Ветераны {$Age['min']} - {$Age['max']} ";}
  if ($AgeABBR == 'M4') {$Age = array('min' => 70, 'max' => 150); $caption_age = "Ветераны 70+ ";}
  if ($AgeABBR == 'O') {$Age = array('min' => 1, 'max' => 150); $caption_age =  "Взрослые ";}
  
  // Изменение заголовка страницы
  drupal_set_title("Рейтинг ФПР среди {$caption} в возрасной категории {$caption_age}", CHECK_PLAIN);
  
  
  // создание временной таблицы "возрастная категория"
  db_query("
    CREATE TEMPORARY TABLE tmp2 SELECT DISTINCT c1.athlete_id, max(c1.wilks) AS wi
    FROM competition AS c1
    INNER JOIN rating_dvfo_athlete AS a1 USING (athlete_id)
    WHERE `date` - age BETWEEN {$Age['min']} AND {$Age['max']}
      AND country = 643
      AND gender = {$gender}
      AND devizion = {$devizion}
    GROUP BY c1.athlete_id
    ORDER BY wi DESC
    LIMIT 302
  ");
  
  
  // выборка данных для HTML таблицы
  $query = db_query("
    SELECT DISTINCT a1.athlete_id, gender, name, age, country, fo, subject_rf, city, 
    devizion, RIGHT(`DATE`, 2) AS year, 
    (date - age) AS age2,
    REPLACE(CONCAT(name_competition.name_ru, age_category.name_ru), '(Взрослые)', '') AS competition,
    weight, squat, brench, deadlift, total, wilks
    FROM rating_dvfo_athlete AS a1 
    INNER JOIN competition AS c1 ON a1.athlete_id = c1.athlete_id
    INNER JOIN tmp2 ON a1.athlete_id = tmp2.athlete_id AND c1.wilks = tmp2.wi
    INNER JOIN name_competition ON c1.competition = name_competition.id
    INNER JOIN age_category ON age_category.id = c1.age_category
    WHERE gender = {$gender}
      AND devizion = {$devizion}
      AND discipline = 'powerlifting'
    ORDER BY NULL, wi DESC
  "); 
  // удаление временной таблицы
  db_query('DROP TABLE tmp2');
  
  
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
  while ($value = $query->fetchAssoc()) {
    $rows[] = array(
      $value['number'] = $i++ . '.',
      array('data' => "<a name={$value['athlete_id']}-{$devizion} href='/athlete/{$value['athlete_id']}'>{$value['name']}</a>",),
      $value['age'] . "/{$value['age2']}",
//      $value['fo'] . '/' . $value['subject_rf'] . '<br />' . $value['city'],
      array('data' => '<strong>' . $value['wilks'] . '</strong>',),
//      $value['total'] . '<br />' . $value['weight'],     
      $value['competition'] . '&nbsp;<i>\'' . $value['year'] . '</i>', 
    ); 
  } 
  
  
  $table = theme('table', array(
                          'header' => $header, 
                          'rows' => $rows, 
                          'attributes' => array('class' => array('MyTable')),
                          'caption' => $caption_table = ($devizion == 1 ? 'Классическое троеборье' : 'Троеборье'),
                          'empty' => 'Таблица пуста. Ищем таланты!',
                        )
              );             
  return $table;      
}


function ratingfpr_page($gender, $AgeABBR) {     
        $eq = html_table_ratingfpr_page($gender, 2, $AgeABBR);
        $raw = html_table_ratingfpr_page($gender, 1, $AgeABBR);
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
        return "
        <table>
                <tr>
				<td colspan='2'>
                    <center>{$AdSenseAdaptive}</center>
                </td>
			    </tr>
               <tr>
               <td style='width: 50%; vertical-align: top'>{$eq}</td>
               <td style='width: 50%; vertical-align: top'>{$raw}</td>
               </tr>      
        </table>";
}