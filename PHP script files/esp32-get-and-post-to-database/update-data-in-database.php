<?php
require 'database.php';

if (isset($_POST["push_up"])) {

  $id_card = $_POST['id_card'];
  $push_up = $_POST['push_up'];

  //........................................ Updating the data in the table.
  $pdo = Database::connect();
  $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  // $sql = "UPDATE trainings SET push_up =  '$push_up' WHERE id_card = '$id_card'";
  $sql = "UPDATE trainings SET push_up = ? WHERE id_card = ?";
  $q = $pdo->prepare($sql);
  $q->execute(array($push_up, $id_card));
  Database::disconnect();
} elseif (isset($_POST["pull_up"])) {

  $id_card = $_POST['id_card'];
  $push_up = $_POST['pull_up'];

  //........................................ Updating the data in the table.
  $pdo = Database::connect();
  $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  // $sql = "UPDATE trainings SET push_up =  '$push_up' WHERE id_card = '$id_card'";
  $sql = "UPDATE trainings SET pull_up = ? WHERE id_card = ?";
  $q = $pdo->prepare($sql);
  $q->execute(array($pull_up, $id_card));
  Database::disconnect();
  //........................................ 
} elseif (isset($_POST["abs"])) {

  $id_card = $_POST['id_card'];
  $abs = $_POST['abs'];

  //........................................ Updating the data in the table.
  $pdo = Database::connect();
  $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  // $sql = "UPDATE trainings SET push_up =  '$push_up' WHERE id_card = '$id_card'";
  $sql = "UPDATE trainings SET abs = ? WHERE id_card = ?";
  $q = $pdo->prepare($sql);
  $q->execute(array($abs, $id_card));
  Database::disconnect();
  //........................................ 
} elseif (isset($_POST["burpee"])) {

  $id_card = $_POST['id_card'];
  $abs = $_POST['burpee'];

  //........................................ Updating the data in the table.
  $pdo = Database::connect();
  $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  // $sql = "UPDATE trainings SET push_up =  '$push_up' WHERE id_card = '$id_card'";
  $sql = "UPDATE trainings SET burpee = ? WHERE id_card = ?";
  $q = $pdo->prepare($sql);
  $q->execute(array($abs, $id_card));
  Database::disconnect();
  //........................................ 
} elseif (isset($_POST["running"])) {

  $id_card = $_POST['id_card'];
  $abs = $_POST['running'];

  //........................................ Updating the data in the table.
  $pdo = Database::connect();
  $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  // $sql = "UPDATE trainings SET push_up =  '$push_up' WHERE id_card = '$id_card'";
  $sql = "UPDATE trainings SET running = ? WHERE id_card = ?";
  $q = $pdo->prepare($sql);
  $q->execute(array($abs, $id_card));
  Database::disconnect();
  //........................................ 
}