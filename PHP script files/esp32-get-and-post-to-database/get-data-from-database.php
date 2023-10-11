<?php
include 'database.php';

$myObj = (object)array();

if (isset($_POST["id_card"])) {
  $id_card = $_POST["id_card"]; // get id_card value from HTTP POST

  //........................................ 
  $pdo = Database::connect();
  $sql = 'SELECT * FROM trainings WHERE id_card="' . $id_card . '"';
  // $sql = 'SELECT * FROM trainings WHERE id_card="8ca6c85b"';
  foreach ($pdo->query($sql) as $row) {
    $myObj->gender = $row['gender'];
    $myObj->height = $row['height'];
    $myObj->service_number = $row['service_number'];
    $myObj->age = $row['age'];

    $myJSON = json_encode($myObj);

    echo $myJSON;
  }


  Database::disconnect();
} else {
  echo "id_card is not set in the HTTP request";
}
  
  //---------------------------------------- 
