<?php

namespace App\Http\Controllers;

use App\Models\Trainings;
use Illuminate\Http\Request;

class TrainingsController extends Controller
{
    public function __construct()
    {
        $this->middleware('auth');
    }

    // Show
    public function index()
    {
    }

    // get id_card from esp32 to register a new player

    public function getidcard()
    {
        $api_url = 'http://192.168.8.153/sending-idcard-from-esp';

        // Read JSON file
        $json_data = file_get_contents($api_url);

        // Decode JSON data into PHP array
        $response_data = json_decode($json_data);

        // All user data exists in 'data' object

        $id_card = $response_data->id_card;
        $PlayerData = Trainings::where('id_card', '=', $id_card)->first();

        if (empty($PlayerData)) {
            return  view("/registerPlayer", ['id_card' => $id_card]);
        } else {

            return redirect("/registerPlayer")->with('fail', 'رقم البطاقة التعريفية ' . $id_card . ' مستخدم بالفعل');
        }
    }


    // create
    public function create()
    {
        return view("registerPlayer", ['id_card' => $id_card = ""]);
    }


    // store
    public function store(Request $request)
    {
        $request->validate(
            [
                'id_card' => 'required',
                'service_number' => 'required|numeric',
                'rank' => 'required',
                'name' => 'required',
                'age' => 'required|numeric|min:18',
                'gender' => 'required',
                'height' => 'required|numeric',
                'weight' => 'required|numeric',
            ],
            [
                'id_card.required' => 'رقم البطاقة التعريفية مطلوب',
                'service_number.required' => 'الرقم العسكري مطلوب',
                'service_number.numeric' => 'يرجي ادخال ارقام فقط',
                'rank.required' => 'الرتبة مطلوبة',
                'name.required' => 'الاسم مطلوب',
                'age.required' => 'السن مطلوب',
                'age.numeric' => 'يرجي ادخال ارقام فقط',
                'age.min' => 'العمر يجب ان يكون 18 على الاقل',
                'gender.required' => 'الجنس مطلوب',
                'height.required' => 'الطول مطلوب',
                'height.numeric' => 'يرجي ادخال ارقام فقط',
                'weight.required' => 'يرجى ادخال الوزن',
                'weight.numeric' => 'يرجي ادخال ارقام فقط',
            ]
        );

        $product = new Trainings;
        $product->id_card           = $request->id_card;
        $product->service_number    = $request->service_number;
        $product->rank              = $request->rank;
        $product->name              = $request->name;
        $product->age               = $request->age;
        $product->gender            = $request->gender;
        $product->height            = $request->height;
        $product->weight            = $request->weight;
        $product->save();


        return  redirect("/registerPlayer")->with('success', 'تم التسجيل بنجاح');
    }
}
