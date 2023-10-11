<?php

namespace App\Exports;

use App\Models\Trainings;
use Maatwebsite\Excel\Concerns\FromCollection;
use Maatwebsite\Excel\Concerns\FromQuery;
use Maatwebsite\Excel\Concerns\WithHeadings;

class ExportTrainings implements FromCollection, WithHeadings

{
    public function collection()
    {
        return Trainings::all();
    }

    public function headings(): array
    {
        return [
            'م',
            'رقم البطاقة التعريفية',
            'الرقم العسكري',
            'الرتبة',
            'الاسم',
            'العمر',
            'النوع',
            'الطول',
            'الوزن',
            'تمرين الضغط',
            'تمرين العقلة',
            'تمرين البطن',
            'الجري المعدل',
            'الضاحية',
            'زمن الضاحية',
            'وقت تسجيل المتسابق',
            'وقت اخر تعديل',
        ];
    }
}
