<?php

namespace App\Http\Controllers;

use App\Models\Trainings;
use Illuminate\Http\Request;

class PlayerDataController extends Controller
{
    /*public function __construct()
    {
        $this->middleware('auth');
    }*/

    // Show
    public function index()
    {
        return view('home');
    }

    // Show Data
    public function show(Request $request)
    {
        $service_number = $request->service_number;
        $PlayerData = Trainings::where('service_number', '=', $service_number)->first();

        if (empty($PlayerData)) {
            return  redirect("/")->with('fail', 'لم يتم العثور على المتسابق');
        } else {
            return view("/home", ['PlayerData' => $PlayerData]);
        }
    }
}
