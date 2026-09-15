<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class AttendanceSnapshot extends Model
{
    protected $fillable = ['present_ids', 'absent_ids'];

    protected $casts = [
        'present_ids' => 'array',
        'absent_ids'  => 'array',
    ];
}
