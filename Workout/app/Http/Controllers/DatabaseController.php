<?php

namespace App\Http\Controllers;

use App\Models\Trainings;
use Illuminate\Http\Request;

class DatabaseController extends Controller
{
    // Show
    public function index()
    {

        $allDatabase = Trainings::orderBy('id', 'desc')->paginate(10);
        return view('databasepage', ['allDatabase' => $allDatabase]);
    }

    // Edit
    public function edit($id)
    {

        $PlayerDataEdit = Trainings::where('id', '=', $id)->first();
        return view("/databasepageEdit", ['PlayerDataEdit' => $PlayerDataEdit]);
    }

    // update
    public function update(Request $request, $id)
    {

        $request->validate(
            [
                'rank' => 'required',
                'name' => 'required',
                'age' => 'required|numeric|min:18',
                'height' => 'required|numeric',
                'weight' => 'required|numeric',
            ],
            [
                'rank.required' => 'الرتبة مطلوبة',
                'name.required' => 'الاسم مطلوب',
                'age.required' => 'السن مطلوب',
                'age.numeric' => 'يرجي ادخال ارقام فقط',
                'age.min' => 'العمر يجب ان يكون 18 على الاقل',
                'height.required' => 'الطول مطلوب',
                'height.numeric' => 'يرجي ادخال ارقام فقط',
                'weight.required' => 'يرجى ادخال الوزن',
                'weight.numeric' => 'يرجي ادخال ارقام فقط',
            ]
        );

        $product = Trainings::findOrFail($id);
        $product->rank              = $request->rank;
        $product->name              = $request->name;
        $product->age               = $request->age;
        $product->height            = $request->height;
        $product->weight            = $request->weight;
        $product->push_up           = $request->push_up;
        $product->pull_up           = $request->pull_up;
        $product->abs               = $request->abs;
        $product->running           = $request->running;
        $product->save();


        return  redirect("/databasepage")->with('success', 'تم التعديل بنجاح');
    }

    // Delete
    public function delete(Request $request, $id)
    {
        $user = Trainings::find($id);
        $user->delete(); //returns true/false
        return  redirect("/databasepage")->with('success', 'تم الحذف بنجاح');
    }

    //search
    public function search(Request $request)
    {
        $service_number = $request->service_number;
        $searchPlayerData = Trainings::where('service_number', '=', $service_number)->first();

        if (empty($searchPlayerData)) {
            return  redirect("/databasepage")->with('fail', 'لم يتم العثور على المتسابق');
        } else {
            $allDatabase = Trainings::orderBy('id', 'desc')->paginate(10);
            return view("/databasepage", ['allDatabase' => $allDatabase, 'searchPlayerData' => $searchPlayerData]);
        }
    }

    public function exportCSVFile()
    {
        return (new ExportTrainings)->download('users.csv', \Maatwebsite\Excel\Excel::CSV);
    }
}
