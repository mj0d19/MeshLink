<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class RollcallState extends Model
{
    protected $table = 'rollcall_states';

    protected $fillable = [
        'checkpoint',
        'present_codes',
        'last_seen_at',
    ];

    protected $casts = [
        'present_codes' => 'array',   // يخليها array مباشرة
        'last_seen_at'  => 'datetime',
    ];
}
