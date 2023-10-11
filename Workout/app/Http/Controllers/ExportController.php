<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Exports\ExportTrainings;
use Maatwebsite\Excel\Facades\Excel;

class ExportController extends Controller
{
    public function index()
    {
        return view('index');
    }

    public function exportXLSXFile()
    {
        return Excel::download(new ExportTrainings, 'بيانات المتسابقين.xlsx');
    }
}
