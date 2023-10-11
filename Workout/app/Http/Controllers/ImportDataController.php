<?php

namespace App\Http\Controllers;

use App\Models\Trainings;
use Illuminate\Support\Facades\Http;

class ImportDataController extends Controller
{

    // // Show
    // public function index()
    // {
    //     return view('importdata');
    // }

    // get Push up counter from esp32
    // public function getpushup()
    // {
    //     $api_url = 'http://192.168.8.199/sending-pushup-counter-from-esp';

    //     // Read JSON file
    //     $json_data = file_get_contents($api_url);

    //     // Decode JSON data into PHP array
    //     $response_data = json_decode($json_data);

    //     // All user data exists in 'data' object

    //     $id_card = $response_data->id_card;
    //     $push_up = $response_data->push_up;

    //     $product = Trainings::where('id_card', $id_card)->first();
    //     $product->push_up         = $push_up;
    //     $product->save();

    //     return  redirect("/importdata")->with('success', 'تم استقبال بيانات تمرين الضغط وتم الحفظ في قاعدة البيانات');
    // }

    // public function getidcard_from_mat()
    // {
    //     $api_url_get = 'http://192.168.8.199/sending-idcard-from-esp';

    //     // Read JSON file
    //     $json_data = file_get_contents($api_url_get);

    //     // Decode JSON data into PHP array
    //     $response_data = json_decode($json_data);

    //     // All user data exists in 'data' object

    //     $id_card = $response_data->id_card;
    //     $PlayerObject = Trainings::where('id_card', '=', $id_card)->first();

    //     // if id_card not present in database, 
    //     if (empty($PlayerObject)) {
    //         // console log: id card not found in database
    //         // $this->info('id card not found in database!!');
    //         return redirect("/importdata")->with('fail', 'رقم البطاقة التعريفية غير موجود في قاعدة البيانات');
    //     } else {
    //         // make a post request on the url with data extracted from playerdata
    //         $height = $PlayerObject->height;
    //         $gender = $PlayerObject->gender;
    //         $service_number = $PlayerObject->service_number;

    //         $response = Http::post('http://192.168.8.199/geting-data-object-from-database', [
    //             'height' => (int)$height,
    //             'gender' => $gender,
    //             'service_number' => $service_number,
    //         ]);
    //         return redirect("/importdata")->with('success', 'تم العثور علي رقم البطاقة التعريفية و تم ارساله الي وحدة التحكم');
    //     }
    // }
}
